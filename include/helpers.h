
/*
 * (c)2026 R van Dorland
 */

#pragma once

#include <Arduino.h>
#include <time.h>

// Functie namen (declaraties)
String formatTime(double decimalTime);
void updateDateTimeStrings(struct tm* timeInfo);
void sensorTask(void* pvParameters);
