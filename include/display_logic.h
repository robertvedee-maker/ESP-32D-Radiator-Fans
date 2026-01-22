/*
 * (c)2026 R van Dorland - Display Logica 
 */

#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <time.h>

// Declaraties van de functies
void setupDisplay();
void drawDisplay(struct tm* timeInfo, time_t now);
