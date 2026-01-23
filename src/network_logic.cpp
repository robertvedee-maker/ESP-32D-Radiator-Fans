/*
 * (c)2026 R van Dorland
 */

#include "network_logic.h"
#include "data_shared.h" // CRUCIAAL: anders kent hij sharedData en dataMutex niet

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
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            sharedData.huidigeFase = OTA_START;
            sharedData.otaProgress = 0;
            xSemaphoreGive(dataMutex);
        }
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            sharedData.huidigeFase = OTA_PROGRESS;
            sharedData.otaProgress = (progress / (total / 100));
            xSemaphoreGive(dataMutex);
        }
    });

    ArduinoOTA.onEnd([]() {
        // Eventueel kort "Update voltooid" tonen, maar ESP herstart direct
    });

    ArduinoOTA.begin();
}
