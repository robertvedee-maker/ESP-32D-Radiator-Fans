/*
 * (c)2026 R van Dorland
 */

#include "data_shared.h"
#include "display_logic.h" // Zorg dat deze bovenaan staat
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

#include "config.h"
#include "daynight.h"
#include "helpers.h"
#include "network_logic.h"
#include "onewire_config.h"
#include "pwm_config.h"
#include "secret.h"

DisplayType u8g2(DISPLAY_ROTATION, DISPLAY_RESET_PIN);

// void drawDisplay(struct tm* timeInfo, time_t now);
void displayTask(void* pvParameters);


// float smoothedTemp = 0.0;
// float TempCFan1 = 0.0, TempCFan2 = 0.0, TempCFan3 = 0.0;
// extern float TempCFan1;
// extern float TempCFan2;
// extern float TempCFan3;
// extern float tempC; // De radiator temperatuur

// // De "echte" opslagplek voor deze strings:
// String sunriseStr = "--:--";
// String sunsetStr = "--:--";
// String currentTimeStr = "--:--:--";
// String currentDateStr = "--. --:---:----";

// Deze functie draait straks op Core 0
void TaskWorkerCore0(void* pvParameters)
{
    // Statische variabelen voor timing binnen deze task
    static unsigned long lastDisplayUpdate = 0;

    for (;;) { // Een oneindige lus (deze task stopt nooit)
        unsigned long currentMillis = millis();

        // Doe alle updates die elke seconde nodig zijn
        if (currentMillis - lastDisplayUpdate >= 1000) {
            lastDisplayUpdate = currentMillis;

            // Haal de tijd op
            time_t now = time(nullptr);
            struct tm* timeInfo = localtime(&now);

            // Alleen actie als tijd geldig is
            if (now > 100000) {
                updateTemperatures(); // Uit onewire_config.cpp
                updateRPMs(); // Uit pwm_config.cpp
                // updateDateTimeStrings(timeInfo); // Uit helpers.cpp
                manageBrightness(); // Uit daynight.cpp
                // updateDisplay(timeInfo, now); // Uit display_logic.cpp
            }
        }

        // Laat de CPU even ademen, voorkomt crashes
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup()
{
    // if (esp_reset_reason() == ESP_RST_POWERON) {
    //     sharedData.huidigeFase = KOUDE_START;
    //     sharedData.faseVervaltijd = millis() + 2000; // 2 sec
    // } else {
    //     sharedData.huidigeFase = INFO_SCHERM; // Software reset
    //     sharedData.faseVervaltijd = millis() + 1000; // 1 sec tonen
    // }


    delay(1000); // Wacht even voor stabiliteit

    Serial.begin(115200);

    dataMutex = xSemaphoreCreateMutex();
    if (dataMutex == NULL) {
        Serial.println("FATALE FOUT: Mutex kon niet worden aangemaakt!");
    }

    // Initialiseer de startfase hier, veilig na de mutex
    if (esp_reset_reason() == ESP_RST_POWERON) {
        sharedData.huidigeFase = KOUDE_START;
        sharedData.faseVervaltijd = millis() + 2000; // 2 sec
    } else {
        // Bij warme start: sla het IP-scherm over of toon het heel kort
        sharedData.huidigeFase = INFO_SCHERM; // Software reset
        sharedData.faseVervaltijd = millis() + 1000; 
    }

    // Start de display-taak op Core 0
    xTaskCreatePinnedToCore(
        displayTask, // Functie die de taak uitvoert (in display_logic.cpp)
        "DisplayTask", // Naam van de taak
        8192, // Stack grootte (ruim voor u8g2)
        NULL, // Parameters
        2, // Prioriteit (hoger dan idle)
        NULL, // Taak handle (niet nodig tenzij je 'm wilt verwijderen)
        0 // PIN NAAR CORE 0
    );

    // Initialiseer Modules
    setupOneWire();
    setupPWM();
    setupDisplay(); // Zet I2C op

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    u8g2.begin();
    u8g2.setContrast(10);

    setupWiFi(SECRET_SSID, SECRET_PASS);
    setupOTA(DEVICE_MDNS_NAME);
    configTzTime(SECRET_TZ_INFO, SECRET_NTP_SERVER);

    fansOn();
    Serial.println("Systeem Modulair Opgestart.");

    // // Start de worker task op Core 0
    // xTaskCreatePinnedToCore(
    //     TaskWorkerCore0, /* Functie naam */
    //     "DisplaySensorTask", /* Naam van de taak (max 16 karakters) */
    //     10000, /* Stack size in bytes */
    //     NULL, /* Parameter (geen) */
    //     1, /* Prioriteit (0 is laagst, 4 is hoogst) */
    //     NULL, /* Task handle (geen nodig) */
    //     0 /* Core ID (0 of 1) */
    // );

    // Serial.println("Systeem Modulair & Dual-Core Opgestart.");
}

void loop()
{
    // Dit blijft op Core 1 (WiFi core) draaien
    ArduinoOTA.handle();

    // Deze loop draait op Core 1 (Pro CPU)
    // Verzamel hier je data en werk de sharedData struct bij (met de mutex!)

    // Voorbeeld van veilig bijwerken van data:
    if (dataMutex != NULL) {
        if (xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) {
            // Update hier sharedData velden (bijv. sharedData.temp_radiator = lees_temp();)
            xSemaphoreGive(dataMutex); // Geef de mutex direct weer vrij
        }
    }

    // ... verder je network_logic_loop() of pwm_config_loop() ...


    // Tijd & Display logica
    unsigned long currentMillis = millis();
    static unsigned long lastDisplayUpdate = 0;

    if (currentMillis - lastDisplayUpdate >= 1000) {
        lastDisplayUpdate = currentMillis;
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);

        if (now > 100000) {
            // updateDateTimeStrings(timeInfo);
            manageBrightness();
            drawDisplay(timeInfo, now); // De nieuwe overzichtelijke aanroep
        }
    }

  
    delay(10);

    // De rest van de loop is nu leeg, want alles zit in TaskWorkerCore0

    // CRUCIAAL: Voorkom dat Core 1 te snel runt en crasht
    vTaskDelay(pdMS_TO_TICKS(10));
}
/*
void loop() {
    ArduinoOTA.handle();

    // Updates
    updateTemperatures();
    updateRPMs();

    // Tijd & Display logica
    unsigned long currentMillis = millis();
    static unsigned long lastDisplayUpdate = 0;

    if (currentMillis - lastDisplayUpdate >= 1000) {
        lastDisplayUpdate = currentMillis;
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);

        if (now > 100000) {
            updateDateTimeStrings(timeInfo);
            manageBrightness();
            drawDisplay(timeInfo, now); // De nieuwe overzichtelijke aanroep
        }
    }

    // Debug naar Serial
    static unsigned long lastLog = 0;
    if (currentMillis - lastLog >= 5000) {
        Serial.printf("Temp: %.2fC | RPM1: %d | FanDuty: %d\n", smoothedTemp, rpms[0], fanDuty);
        lastLog = currentMillis;
    }
}
*/

/*
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
    u8g2.setFont(u8g2_font_mozart_nbp_tf);
    u8g2.drawStr(32, 10, "RADIATOR MONITOR");
    u8g2.drawLine(0, 13, 128, 13);

    // Grote Temperatuur Weergave
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(0, 40);
    // u8g2.print(tempC, 1); // Toon 1 decimaal
    u8g2.print(smoothedTemp, 1); // Toon 1 decimaal
    u8g2.print("ºC"); // Dankzij enableUTF8Print()


    // PWM Balk midden
    int pwmPercent = map(fanDuty, 0, 1023, 0, 100);
    u8g2.drawFrame(0, 56, 128, 8);
    u8g2.drawBox(2, 58, map(pwmPercent, 0, 100, 0, 124), 4);
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 53, "FAN POWER:");
    u8g2.setCursor(110, 53);
    u8g2.print(pwmPercent);
    u8g2.print("%");

    // RPM Links
    u8g2.setFont(u8g2_font_mozart_nbp_tf);
    u8g2.setCursor(0, 80);
    u8g2.print("Fan 1:");
    u8g2.print(rpms[0]);
    u8g2.setCursor(0, 92);
    u8g2.print("Fan 2:");
    u8g2.print(rpms[1]);
    u8g2.setCursor(0, 104);
    u8g2.print("Fan 3:");
    u8g2.print(rpms[2]);

    TempCFan1 = sensors.getTempCByIndex(0); // tempC fan Air links
    TempCFan2 = sensors.getTempCByIndex(1); // tempC fan Middel
    TempCFan3 = sensors.getTempCByIndex(2); // tempC fan Air rechts

    // tempC fan Air rechts
    u8g2.setCursor(74, 80);
    u8g2.print(TempCFan1, 1);u8g2.print("ºC");
    u8g2.setCursor(74, 92);
    u8g2.print(TempCFan2, 1);u8g2.print("ºC");
    u8g2.setCursor(74, 104);
    u8g2.print(TempCFan3, 1);u8g2.print("ºC");

    // datum en tijd onderaan
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.setCursor(0, 126);
    u8g2.print(currentDateStr);
    u8g2.setCursor(90, 126);
    u8g2.print(currentTimeStr);



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

    delay(5000); // Update elke 5 seconden
    u8g2.sendBuffer();
}
*/
