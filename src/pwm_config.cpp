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
volatile int pulseCount1 = 0, pulseCount2 = 0, pulseCount3 = 0;

void IRAM_ATTR countPulses1() { pulseCount1++; }
void IRAM_ATTR countPulses2() { pulseCount2++; }
void IRAM_ATTR countPulses3() { pulseCount3++; }

void setupPWM()
{
    pinMode(pwrFanPin, OUTPUT);
    pinMode(tachoPin1, INPUT_PULLUP);
    pinMode(tachoPin2, INPUT_PULLUP);
    pinMode(tachoPin3, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(tachoPin1), countPulses1, FALLING);
    attachInterrupt(digitalPinToInterrupt(tachoPin2), countPulses2, FALLING);
    attachInterrupt(digitalPinToInterrupt(tachoPin3), countPulses3, FALLING);

    // Let op: In ESP32 Arduino Core 3.x is ledcSetup veranderd,
    // maar als dit voor jou werkt houden we het zo.
    ledcSetup(pwmChannel, pwmFreq, pwmRes);
    ledcAttachPin(fanPwmPin, pwmChannel);
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

