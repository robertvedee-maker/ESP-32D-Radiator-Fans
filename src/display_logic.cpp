/*
 * (c)2026 R van Dorland - Display Logica
 */

#include "display_logic.h"
#include "data_shared.h"
#include "config.h"
#include <U8g2lib.h>
#include <Arduino.h>
#include <WiFi.h> // Nodig voor IP-adres
#include <esp_system.h> // Deze is nodig voor esp_reset_reason()

// Referentie naar het u8g2 object in config.h
extern DisplayType u8g2;

// --- Helper Functies voor de Schermen ---

// Tekent het opstartscherm (bv. 2 seconden)
void drawStartScreen(const SensorData& d) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 15, "Systeem Start...");
    u8g2.drawStr(0, 30, d.currentTimeStr);
}

// Tekent het netwerk-informatiescherm (bv. 5 seconden)
void drawInfoScreen(const SensorData& d) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Netwerk Info:");
    u8g2.drawStr(0, 25, WiFi.localIP().toString().c_str());
    u8g2.drawStr(0, 40, DEVICE_MDNS_NAME);
    
    // // Optioneel: toon RSSI of NTP status uit de struct
    // char buf[20];
    // snprintf(buf, sizeof(buf), "RSSI: %ld dBm", d.rssiIcon); 
    // u8g2.drawStr(0, 55, buf);
}

// Tekent het hoofddashboard (fans, temps, etc)
void drawDashboard(const SensorData& d) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
        // Optioneel: toon RSSI of NTP status uit de struct
    char buf[20];
    snprintf(buf, sizeof(buf), "RSSI: %ld dBm", d.rssiIcon); 
    u8g2.drawStr(0, 55, buf);
    u8g2.drawStr(0, 10, d.currentTimeStr);

    // Voeg hier de rest van je fans/data toe
}

// --- Hoofd Teken Functie (De State Machine) ---

void drawDisplay(struct tm* timeInfo, time_t now) {
    // 1. Lokale kopie/alias voor leesbaarheid
    static SensorData displayData; 
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        displayData = sharedData;
        xSemaphoreGive(dataMutex);
    }
    auto& d = displayData; // De korte alias 'd'

    // 2. FASE LOGICA (Niet-blokkerend met timer)
    // Deze logica moet eigenlijk in de TaskWorkerCore0 of loop() draaien, 
    // niet in de tekencode zelf, omdat de tekencode alleen de staat moet 'lezen'.
    // Voor nu laten we het hier om het simpel te houden:
    if (millis() > d.faseVervaltijd && d.huidigeFase != DASHBOARD) {
         if (d.huidigeFase == KOUDE_START) {
            sharedData.huidigeFase = INFO_SCHERM;
            sharedData.faseVervaltijd = millis() + 5000; // 5 sec info
        } else if (d.huidigeFase == INFO_SCHERM) {
            sharedData.huidigeFase = DASHBOARD;
        }
    }

    // 3. Teken het juiste scherm
    u8g2.clearBuffer();
    switch (d.huidigeFase) {
        case KOUDE_START:
            drawStartScreen(d);
            break;
        case INFO_SCHERM:
            drawInfoScreen(d);
            break;
        case DASHBOARD:
            drawDashboard(d);
            break;
    }
    u8g2.sendBuffer();
}

// De display taak (apart op Core 0)
void displayTask(void* pvParameters) {
    for (;;) {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        if (now > 100000) {
            drawDisplay(timeInfo, now);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Teken 10x per seconde
    }
}

void setupDisplay() {
    // Eventuele setup code indien nodig
}
