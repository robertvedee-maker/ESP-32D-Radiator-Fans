/*
 * (c)2026 R van Dorland Netwerk functies
 */

#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

// Functie declaraties
void toonNetwerkInfo();
void setupWiFi(const char* ssid, const char* password);
void setupOTA(const char* hostname);
