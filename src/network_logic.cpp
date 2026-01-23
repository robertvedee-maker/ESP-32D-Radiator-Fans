/*
 * (c)2026 R van Dorland
 */

#include "network_logic.h"
#include "helpers.h"
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <esp_system.h>

void setupWiFi(const char* ssid, const char* password)
{
    // WiFi.setSleep(false);
    WiFi.begin(ssid, password);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
}

void setupOTA(const char* hostname)
{
    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.onStart([]() {
        // zie inhoud display_logic.cpp ->
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        // zie inhoud display_logic.cpp ->
    });

    ArduinoOTA.begin();
}
