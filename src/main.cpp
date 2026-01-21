#include "daynight.h"
#include "network_logic.h" // Volgorde is hier erg belangrijk. niet aanpassen!
#include "secret.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>
#include <helpers.h>
#include <math.h> // Nodig voor log() berekening

// U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE); // OLED 1.50 128x128
// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //OLED 1.54 128x64
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE); // OLED 1.30 128x64
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE); // OLED 1.54 128x64

// End of constructor list

bool eersteStart = true; // Zorgt ervoor dat info éénmalig getoond wordt

String sunriseStr = "--:--";
String sunsetStr = "--:--";
String currentTimeStr = "--:--:--";
String currentDateStr = "--. --:---:----";
String TempC = "0.0";
float tempC = 0.0;
float minTemp = 20.0;
float maxTemp = 80.0;
int fanDuty = 0;
int rawValue = 0;

int rpms[3] = { 0, 0, 0 }; // Array om de RPM waarden op te slaan

// De "belofte" aan de compiler dat deze functie verderop staat:
void drawDisplay(struct tm* timeInfo, time_t now);

unsigned long lastBrightnessCheck = 0;
const unsigned long brightnessInterval = 60000; // 1 minuut

// Pin definities
const int fanPwmPin = 13; // GPIO13 voor PWM output naar ventilator
const int tachoPin1 = 25; // GPIO25 voor Tachometer (met 10k pull-up!)
const int tachoPin2 = 26; // GPIO26 voor Tachometer (met 10k pull-up!)
const int tachoPin3 = 27; // GPIO27 voor Tachometer (met 10k pull-up!)
const int pwrFanPin = 33; // GPIO33 om 12V voeding van de fan te schakelen (optioneel)
const int oneWireBus = 15; // GPIO15 voor 1-Wire DS18B20

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// --- NTC & DIVIDER PARAMETERS ---
const float seriesResistor = 10000; // Vaste weerstand van 10k
const float nominalResistance = 11500; // NTC weerstand bij 25 graden
const float nominalTemperature = 25; // Nominale temp in Celsius
const float bCoefficient = 3950; // Beta-waarde van de meeste 10k NTC's
const float adcMax = 4095.0; // 12-bit ADC resolutie van de C3

float smoothedTemp = 0.0; // De gefilterde temperatuur

// PWM instellingen voor 4-pin ventilator
const int pwmFreq = 25000; // 25 kHz (industriestandaard)
const int pwmChannel = 0; // Gebruik PWM kanaal 0
const int pwmRes = 10; // 10-bit resolutie (0-1023)

// Tachometer variabelen
volatile int pulseCount1 = 0;
volatile int pulseCount2 = 0;
volatile int pulseCount3 = 0;
unsigned long lastMillis = 0;
int currentRPM = 0;

volatile unsigned long lastPulseTime = 0;
const int debounceTime = 2; // milliseconden (negeer pulsen sneller dan ~15.000 RPM)

// Interrupt Service Routine (moet in RAM voor snelheid)
void IRAM_ATTR countPulses1()
{
    unsigned long now = millis();
    if (now - lastPulseTime > debounceTime) {
        pulseCount1++;
        lastPulseTime = now;
    }
}

void IRAM_ATTR countPulses2()
{
    unsigned long now = millis();
    if (now - lastPulseTime > debounceTime) {
        pulseCount2++;
        lastPulseTime = now;
    }
}

void IRAM_ATTR countPulses3()
{
    unsigned long now = millis();
    if (now - lastPulseTime > debounceTime) {
        pulseCount3++;
        lastPulseTime = now;
    }
}

void fansOff()
{
    digitalWrite(pwrFanPin, LOW); // MOSFET uit
    // Ontkoppel de PWM-sturing van de pin (maak de pin 'zwevend')
    ledcDetachPin(fanPwmPin);
    pinMode(fanPwmPin, INPUT);
}

void fansOn()
{
    pinMode(fanPwmPin, OUTPUT);
    ledcAttachPin(fanPwmPin, pwmChannel);
    digitalWrite(pwrFanPin, HIGH); // MOSFET aan
}

/*
 * Setup
 */
void setup()
{
    pinMode(fanPwmPin, INPUT);
    // Start met een veilige lage snelheid (ca. 10%)
    // ledcWrite(pwmChannel, 50);

    // Configureer PWM kanaal
    ledcSetup(pwmChannel, pwmFreq, pwmRes);
    ledcAttachPin(fanPwmPin, pwmChannel);

    //  Sinds versie 3.x van de ESP32 Arduino Core is de manier waarop PWM (ledc) werkt veranderd:
    //  Oude methode: ledcSetup() en ledcAttachPin().
    //  Nieuwe methode (2025/2026): Gebruik direct ledcAttach(pin, freq, resolution) en daarna ledcWrite(pin, duty).
    //  Voor compatibiliteit met oudere code gebruiken we hier de oude methode.
    //  Raadpleeg de documentatie van de ESP32 Arduino Core voor meer details.
    //  Zie: https://github.com/espressif/arduino-esp32/blob/master/docs/ledc.rst
    //  Voor nu blijft de oude methode in gebruik voor bredere compatibiliteit.

    // Tachometer setup
    pinMode(tachoPin1, INPUT_PULLUP); // Extra veiligheid naast externe pull-up
    attachInterrupt(digitalPinToInterrupt(tachoPin1), countPulses1, FALLING);

    pinMode(tachoPin2, INPUT_PULLUP); // Extra veiligheid naast externe pull-up
    attachInterrupt(digitalPinToInterrupt(tachoPin2), countPulses2, FALLING);

    pinMode(tachoPin3, INPUT_PULLUP); // Extra veiligheid naast externe pull-up
    attachInterrupt(digitalPinToInterrupt(tachoPin3), countPulses3, FALLING);

    // 1. Hardware basis
    Serial.begin(115200);
    sensors.begin();
    Wire.begin(I2C_SDA, I2C_SCL);
    u8g2.begin();
    u8g2.setContrast(10);

    // 2. Netwerk (nu lekker kort!)
    setupWiFi(SECRET_SSID, SECRET_PASS);

    // fetchWeather(); // Haal direct het eerste weerbericht op

    if (WiFi.status() == WL_CONNECTED) {
        toonNetwerkInfo(); // Deze functie bevat de 'rtc_info->reason' check
    }

    setupOTA(DEVICE_MDNS_NAME);

    // 3. Tijd en Regeling
    configTzTime(SECRET_TZ_INFO, SECRET_NTP_SERVER);

    // Initialiseer eerste waarden
    manageBrightness();

    // 1. Lees NTC en stuur fan aan
    // rawValue = analogRead(ntcPin);
    // tempC = calculateCelsius(rawValue);
    sensors.requestTemperatures();
    float dsTempC0 = sensors.getTempCByIndex(0);
    float dsTempC1 = sensors.getTempCByIndex(1);
    float dsTempC2 = sensors.getTempCByIndex(2);
    float dsTempC3 = sensors.getTempCByIndex(3);
    tempC = dsTempC0;

    // Succes schermpje (optioneel)
    const char* Msg = "Systeem Online";
    u8g2.clearBuffer();
    u8g2.drawRFrame(0, 0, LCDWidth, LCDHeight, 5);
    u8g2.drawStr(ALIGN_CENTER(Msg), ALIGN_V_CENTER, Msg);
    // u8g2.print(WiFi.localIP().toString());
    u8g2.sendBuffer();
    delay(1000);
    fansOn();
}

/*
 *loop
 */
void loop()
{
    // 1. Altijd als eerste: Onderhoud voor OTA en Netwerk
    ArduinoOTA.handle();

    sensors.requestTemperatures();
    float dsTempC0 = sensors.getTempCByIndex(0);
    Serial.print("Temperatuur: ");
    Serial.print(dsTempC0);
    Serial.print("ºC | ");
    float dsTempC1 = sensors.getTempCByIndex(1);
    Serial.print("Temperatuur: ");
    Serial.print(dsTempC1);
    Serial.print("ºC | ");
    float dsTempC2 = sensors.getTempCByIndex(2);
    Serial.print("Temperatuur: ");
    Serial.print(dsTempC2);
    Serial.print("ºC | ");
    float dsTempC3 = sensors.getTempCByIndex(3);
    Serial.print("Temperatuur: ");
    Serial.print(dsTempC3);
    Serial.println("ºC");
    delay(500);

    unsigned long currentMillis = millis();

    // // 2. NTC-update timer (elke 15 minuten = 900.000 ms)
    // static unsigned long lastNTC_Update = 0;
    // const unsigned long NTC_Interval = 900000;

    // if (currentMillis - lastNTC_Update >= NTC_Interval || lastNTC_Update == 0) {
    //     lastNTC_Update = currentMillis;
    //     // Lees NTC en stuur fan aan
    //     rawValue = analogRead(ntcPin);
    //     tempC = calculateCelsius(rawValue);
    // }

    // 3. Display en Tijd update timer (elke seconde = 1000 ms)
    static unsigned long lastDisplayUpdate = 0;
    if (currentMillis - lastDisplayUpdate >= 1000) {
        lastDisplayUpdate = currentMillis;

        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);

        // Alleen actie ondernemen als we een geldige tijd hebben (na 1 jan 1970)
        if (now > 100000) {
            // Update de tijds-strings (Zo. 12 Jan, etc.)
            updateDateTimeStrings(timeInfo);

            // Controleer elke minuut de helderheid/zonnestand
            static unsigned long lastBrightnessCheck = 0;
            if (currentMillis - lastBrightnessCheck >= 60000) {
                lastBrightnessCheck = currentMillis;
                manageBrightness();
            }

            // Teken alles op het scherm (Klok, Datum, Iconen en Weer)
            drawDisplay(timeInfo, now);
        }
    }
    // timeClient.update(); // Update time from NTP (if needed)
    // unsigned long currentTime = timeClient.getEpochTime(); // Get current Unix time

    // rawValue = analogRead(ntcPin);
    rawValue = dsTempC3; // Voor NTC simulatie met DS18B20
    // float rawTemp = calculateCelsius(rawValue); // Lees de temperatuur van de NTC sensor
    float rawTemp = sensors.getTempCByIndex(3); // Lees de temperatuur van de DS18B20 sensor

    if (smoothedTemp == 0.0) {
        smoothedTemp = rawTemp;
    } else {
        // Hoe hoger de 0.98, hoe trager/rustiger de reactie
        smoothedTemp = (smoothedTemp * 0.98) + (rawTemp * 0.02);
    }
    tempC = smoothedTemp;

    // Aansturing (ADC): Warmer = lagere ADC = hogere PWM
    // fanDuty = map(rawValue, 1800, 800, 0, 1023); // Pas deze waarden aan op jouw NTC en ventilator 2200 = koud, 1200 = warm
    fanDuty = map(smoothedTemp, minTemp, maxTemp, 0, 1023);
    fanDuty = constrain(fanDuty, 0, 1023);
    ledcWrite(pwmChannel, fanDuty);

    // RPM berekening

    if (millis() - lastMillis >= 1000) {
        unsigned long duration = millis() - lastMillis;
        lastMillis = millis();

        // Veilig de tellers kopiëren en resetten
        noInterrupts();
        int counts[3] = { pulseCount1, pulseCount2, pulseCount3 };
        pulseCount1 = 0;
        pulseCount2 = 0;
        pulseCount3 = 0;
        interrupts();

        // Bereken RPM voor alle drie de fans
        for (int i = 0; i < 3; i++) {
            rpms[i] = (counts[i] / 2.0) * (60000.0 / duration);
        }

        // Nu kun je rpms[0], rpms[1] en rpms[2] gebruiken voor je scherm of serieel
        Serial.printf("Fan1: %d | Fan2: %d | Fan3: %d RPM\n", rpms[0], rpms[1], rpms[2]);
    }
}

void drawDisplay(struct tm* timeInfo, time_t now)
{
    u8g2.clearBuffer();
    u8g2.enableUTF8Print();

    // --- 1. Bovenste balk: Iconen & Datum ---
    bool timeValid = (now > 1735689600);
    long ntpIcon = timeValid ? 57367 : 57368;
    long rssiIcon = map(WiFi.RSSI(), -90, -30, 57949, 57953);

    u8g2.setFont(u8g2_font_waffle_t_all); // Zorg dat dit font actief is
    u8g2.drawGlyph(0, 10, ntpIcon); // Y iets verlaagd naar 10 voor betere weergave
    u8g2.drawGlyph(14, 10, rssiIcon); // X iets meer ruimte gegeven

    // Header
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(32, 10, "RADIATOR MONITOR");
    u8g2.drawLine(0, 13, 128, 13);

    // Grote Temperatuur Weergave
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(0, 40);
    // u8g2.print(tempC, 1); // Toon 1 decimaal
    u8g2.print(smoothedTemp, 1); // Toon 1 decimaal
    u8g2.print("ºC"); // Dankzij enableUTF8Print()

    // RPM Rechtsboven
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.setCursor(80, 28);
    u8g2.print(rpms[0]);
    u8g2.setCursor(80, 40);
    u8g2.print(rpms[1]);
    u8g2.setCursor(80, 52);
    u8g2.print(rpms[2]);
    // PWM Balk onderin
    int pwmPercent = map(fanDuty, 0, 1023, 0, 100);
    u8g2.drawFrame(0, 56, 128, 8);
    u8g2.drawBox(2, 58, map(pwmPercent, 0, 100, 0, 124), 4);
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 53, "FAN POWER:");
    u8g2.setCursor(110, 53);
    u8g2.print(pwmPercent);
    u8g2.print("%");

    u8g2.sendBuffer();
    // delay(200);

    // // 3. Output naar Seriële Monitor
    // Serial.print("NTC: ");
    // Serial.print(rawValue);
    // Serial.print(" | PWM: ");
    // Serial.print((fanDuty / 1023.0) * 100);
    // Serial.print("% | Fan Snelheid: ");
    // Serial.print(rpm);
    // Serial.println(" RPM");

    delay(2000); // Update elke 2 seconden
    u8g2.sendBuffer();
}
