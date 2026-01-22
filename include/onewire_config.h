/*
 * (c)2026 R van Dorland - OneWire Configuraties voor DS18B20 sensoren
 */

#pragma once
#include <DallasTemperature.h>
#include <OneWire.h>

extern const int oneWireBus;
extern OneWire oneWire;             // EXTERN toevoegen
extern DallasTemperature sensors;   // EXTERN toevoegen

void setupOneWire();
void updateTemperatures();

extern float TempCFan1;
extern float TempCFan2;
extern float TempCFan3;
extern float tempC; // De radiator temperatuur
