/*
 * (c)2026 R van Dorland Netwerk functies
 */

#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

// Extern objecten en variabelen (gedefinieerd in main.cpp)
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
extern String sunriseStr;
extern String sunsetStr;
extern String currentTimeStr;
extern String currentDateStr;
extern bool eersteStart;
extern unsigned long lastBrightnessCheck;

// Constanten omgezet naar constexpr
// Let op: deze moeten in de header staan als ze overal gebruikt worden
extern const unsigned long brightnessInterval;
// constexpr int LCDWidth = 128;
// constexpr int LCDHeight = 64;
// constexpr int ALIGN_V_CENTER = 32;

// Functie declaraties
void toonNetwerkInfo();
void setupWiFi(const char* ssid, const char* password);
void setupOTA(const char* hostname);
