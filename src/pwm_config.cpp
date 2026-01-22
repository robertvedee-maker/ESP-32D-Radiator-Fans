/*
 * (c)2026 R van Dorland - PWM Configuraties voor ventilatoren en toerenmeting
 */

#include "pwm_config.h"
#include <Arduino.h>


const int fanPwmPin = 13;
const int tachoPin1 = 25;
const int tachoPin2 = 26;
const int tachoPin3 = 27;
const int pwrFanPin = 33;

// PWM instellingen (ESP32 Core 3.x stijl)
const int pwmFreq = 25000;
const int pwmRes = 10;
const int pwmChannel = 0;

volatile int pulseCount1 = 0, pulseCount2 = 0, pulseCount3 = 0;
unsigned long lastPulseTime = 0;
unsigned long lastRPMCalc = 0;
int rpms[3] = {0, 0, 0};
int fanDuty = 0;

void IRAM_ATTR countPulses1() { pulseCount1++; }
void IRAM_ATTR countPulses2() { pulseCount2++; }
void IRAM_ATTR countPulses3() { pulseCount3++; }

void setupPWM() {
    pinMode(pwrFanPin, OUTPUT);
    pinMode(tachoPin1, INPUT_PULLUP);
    pinMode(tachoPin2, INPUT_PULLUP);
    pinMode(tachoPin3, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(tachoPin1), countPulses1, FALLING);
    attachInterrupt(digitalPinToInterrupt(tachoPin2), countPulses2, FALLING);
    attachInterrupt(digitalPinToInterrupt(tachoPin3), countPulses3, FALLING);

    ledcSetup(pwmChannel, pwmFreq, pwmRes);
    ledcAttachPin(fanPwmPin, pwmChannel);
}

void fansOff() {
    digitalWrite(pwrFanPin, LOW);
    ledcDetachPin(fanPwmPin);
}

void fansOn() {
    ledcAttachPin(fanPwmPin, pwmChannel);
    digitalWrite(pwrFanPin, HIGH);
}

void updateRPMs() {
    unsigned long now = millis();
    if (now - lastRPMCalc >= 1000) {
        // Berekening (2 pulsen per omwenteling)
        rpms[0] = (pulseCount1 * 60) / 2;
        rpms[1] = (pulseCount2 * 60) / 2;
        rpms[2] = (pulseCount3 * 60) / 2;

        pulseCount1 = pulseCount2 = pulseCount3 = 0;
        lastRPMCalc = now;
    }
}
