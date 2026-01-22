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

// Functies
void setupPWM();
void updateRPMs();
void fansOff();
void fansOn();

// Globalen
extern int rpms[3];
extern int fanDuty;
