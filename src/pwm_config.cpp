/*
 * (c)2026 R van Dorland - PWM Configuraties voor ventilatoren en toerenmeting
 */

#include "pwm_config.h"
#include "data_shared.h" // CRUCIAAL: anders kent hij sharedData en dataMutex niet
#include <Arduino.h>

const int fanPwmPin = 13;
const int tachoPin1 = 25;
const int tachoPin2 = 26;
const int tachoPin3 = 27;
const int pwrFanPin = 33;

// PWM instellingen
const int pwmFreq = 25000;
const int pwmRes = 10;
const int pwmChannel = 0;

// Volatile tellers voor de interrupts
volatile unsigned long pulseCount1 = 0;
volatile unsigned long pulseCount2 = 0;
volatile unsigned long pulseCount3 = 0;

// We gebruiken unsigned long voor micros()
volatile unsigned long lastMicros1 = 0;
volatile unsigned long lastMicros2 = 0;
volatile unsigned long lastMicros3 = 0;

// De debounce tijd: 2000 micros = 2ms (komt overeen met max 15.000 RPM)
const unsigned long debounceTime = 10000; 

void IRAM_ATTR countPulses1() {
    unsigned long now = micros();
    if (now - lastMicros1 > debounceTime) {
        pulseCount1 = pulseCount1 + 1; // Compiler-vriendelijke optelling
        lastMicros1 = now;
    }
}

void IRAM_ATTR countPulses2() {
    unsigned long now = micros();
    if (now - lastMicros2 > debounceTime) {
        pulseCount2 = pulseCount2 + 1;
        lastMicros2 = now;
    }
}

void IRAM_ATTR countPulses3() {
    unsigned long now = micros();
    if (now - lastMicros3 > debounceTime) {
        pulseCount3 = pulseCount3 + 1;
        lastMicros3 = now;
    }
}









// volatile int pulseCount1 = 0, pulseCount2 = 0, pulseCount3 = 0;

// void IRAM_ATTR countPulses1() { pulseCount1 += 1; }
// void IRAM_ATTR countPulses2() { pulseCount1 += 1; }
// // void IRAM_ATTR countPulses3() { pulseCount1 += 1; }

// void IRAM_ATTR countPulses1() { pulseCount1 = pulseCount1 + 1; }
// void IRAM_ATTR countPulses2() { pulseCount2 = pulseCount2 + 1; }
// void IRAM_ATTR countPulses3() { pulseCount3 = pulseCount3 + 1; }

// void IRAM_ATTR countPulses1() { pulseCount1++; }
// void IRAM_ATTR countPulses2() { pulseCount2++; }
// void IRAM_ATTR countPulses3() { pulseCount3++; }

void setupPWM()
{
    pinMode(pwrFanPin, OUTPUT);
    pinMode(tachoPin1, INPUT_PULLUP);
    pinMode(tachoPin2, INPUT_PULLUP);
    pinMode(tachoPin3, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(tachoPin1), countPulses1, FALLING);
    attachInterrupt(digitalPinToInterrupt(tachoPin2), countPulses2, FALLING);
    attachInterrupt(digitalPinToInterrupt(tachoPin3), countPulses3, FALLING);

    // Let op: In ESP32 Arduino Core 3.x is ledcSetup veranderd naar ledcAttach
    ledcAttach(fanPwmPin, pwmFreq, pwmRes);


    
}



void fansOff()
{
    digitalWrite(pwrFanPin, LOW);
}

void fansOn()
{
    digitalWrite(pwrFanPin, HIGH);
}

void updateRPMs()
{
    static unsigned long lastMillis = 0;
    unsigned long currentMillis = millis();

    // Bereken elke seconde
    if (currentMillis - lastMillis >= 1000) {
        float timeElapsed = (currentMillis - lastMillis) / 1000.0;

        // 1. Kopieer pulsen en zet ze direct op 0 (minimaliseert misgelopen pulsen)
        noInterrupts(); // Even interrupts uit voor een schone kopie
        int p1 = pulseCount1;
        pulseCount1 = 0;
        int p2 = pulseCount2;
        pulseCount2 = 0;
        int p3 = pulseCount3;
        pulseCount3 = 0;
        interrupts();

        // 2. Bereken RPM (gaat uit van 2 pulsen per omwenteling)
        int rpm1 = (p1 * 60) / 2;
        int rpm2 = (p2 * 60) / 2;
        int rpm3 = (p3 * 60) / 2;

        lastMillis = currentMillis;

        // 3. Veilig wegschrijven naar de Mutex
        if (dataMutex != NULL && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            sharedData.fanRPM1 = rpm1;
            sharedData.fanRPM2 = rpm2;
            sharedData.fanRPM3 = rpm3;
            xSemaphoreGive(dataMutex);
        }
    }
}

void setFanSpeed() {
    float maxTemp = 0.0;
    
    // 1. Haal de hoogste temperatuur op uit sharedData
    if (dataMutex != NULL && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        // We kijken naar de fans, maar je zou hier ook sharedData.tempC (radiator) kunnen meenemen
        maxTemp = max(sharedData.tempCFan1, sharedData.tempCFan2);
        maxTemp = max(maxTemp, sharedData.tempCFan3);
        maxTemp = max(maxTemp, sharedData.tempC); // Inclusief radiator sensor
        xSemaphoreGive(dataMutex);
    }

    // 2. Regel-logica (Temperaturen aanpassen naar wens)
    int pwmValue = 0;
    const float tempMin = 28.0; // Fans gaan aan bij 28 graden
    const float tempMax = 70.0; // Fans op 100% bij 70 graden

    if (maxTemp < tempMin) {
        pwmValue = 0; 
        fansOff(); // Zet de M-FET ook uit voor totale stilte
    } else {
        fansOn();  // Zet de M-FET aan
        // Map de temperatuur naar PWM (0-1023 omdat je 10-bit resolutie gebruikt!)
        pwmValue = map(maxTemp * 10, tempMin * 10, tempMax * 10, 10, 1023); 
        pwmValue = constrain(pwmValue, 10, 1023); // 300 is vaak de minimale draaisnelheid
    }

    // 3. Stuur de fans aan
    ledcWrite(fanPwmPin, pwmValue);

    // 4. Update het percentage voor je display
    if (dataMutex != NULL && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        sharedData.pwmPercentage = map(pwmValue, 10, 1023, 0, 100);
        xSemaphoreGive(dataMutex);
    }
}
