/*
 * (c)2026 R van Dorland - PWM Configuraties voor ventilatoren en toerenmeting
 */

#pragma once
#include <Arduino.h>

// Pinnen
extern const int fanPwmPin;
extern const int tachoPin1;
extern const int tachoPin2;
extern const int tachoPin3;
extern const int pwrFanPin;

// PWM instellingen
extern const int pwmFreq;
extern const int pwmRes;
extern const int pwmChannel;
// Temperatuur instellingen
extern const float minTemp;
extern const float maxTemp;
extern float tempC;
extern float smoothedTemp;

// Functies
void updateRPMs();
void setupPWM();
void setFanSpeed();
void fansOff();
void fansOn();

// Globalen
extern int rpms[3];
extern int fanDuty;
