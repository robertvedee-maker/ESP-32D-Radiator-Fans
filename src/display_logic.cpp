/*
 * (c)2026 R van Dorland - Display Logica
 */

#include "display_logic.h"
#include "config.h"
#include "data_shared.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h> // Nodig voor IP-adres
#include <esp_system.h> // Deze is nodig voor esp_reset_reason()

// Referentie naar het u8g2 object in config.h
extern DisplayType u8g2;

// --- Helper Functies voor de Schermen ---
inline u8g2_uint_t GET_LCD_WIDTH() { return u8g2.getDisplayWidth(); }
inline u8g2_uint_t GET_LCD_HEIGHT() { return u8g2.getDisplayHeight(); }
// 1. Horizontale uitlijning
inline u8g2_uint_t ALIGN_CENTER(const char* t) { return (u8g2_uint_t)((u8g2.getDisplayWidth() - u8g2.getUTF8Width(t)) / 2); }
inline u8g2_uint_t ALIGN_RIGHT(const char* t) { return (u8g2_uint_t)(u8g2.getDisplayWidth() - u8g2.getUTF8Width(t)); }
// 2. Verticale uitlijning
inline u8g2_uint_t GET_CHAR_HEIGHT() { return (u8g2_uint_t)u8g2.getMaxCharHeight(); }
inline u8g2_uint_t ALIGN_V_CENTER() { return (u8g2_uint_t)((u8g2.getDisplayHeight() + u8g2.getMaxCharHeight()) / 2); }
// 3. Vaste posities
constexpr u8g2_uint_t ALIGN_LEFT = 0;
inline u8g2_uint_t ALIGN_TXT_RIGHT(const char* t) { return (u8g2.getUTF8Width(t)) / 2; }

// We berekenen ALIGN_BOTTOM live via het object
inline u8g2_uint_t ALIGN_BOTTOM() { return (u8g2_uint_t)u8g2.getDisplayHeight(); }

// Tekent het opstartscherm (bv. 2 seconden)
void drawStartScreen(const SensorData& d)
{
    u8g2.setFont(u8g2_font_6x10_tr);
    String Msg = "Systeem Start...";
    u8g2.drawStr(ALIGN_CENTER(Msg.c_str()), ALIGN_V_CENTER(), Msg.c_str());
    // u8g2.drawStr(0, 30, d.currentTimeStr);
}

// Tekent het netwerk-informatiescherm (bv. 5 seconden)
void drawInfoScreen(const SensorData& d)
{
    u8g2.setFont(u8g2_font_6x10_tr);
    int yPos = ALIGN_V_CENTER() - 10;
    String Msg1 = "Netwerk Info:";
    u8g2.drawStr(ALIGN_CENTER(Msg1.c_str()), yPos, Msg1.c_str());
    yPos += u8g2.getMaxCharHeight() + 2;
    String Msg2 = "IP: " + WiFi.localIP().toString();
    u8g2.drawStr(ALIGN_CENTER(Msg2.c_str()), yPos, Msg2.c_str());
    yPos += u8g2.getMaxCharHeight() + 2;
    String Msg3 = "mDNS: " + String(DEVICE_MDNS_NAME) /* + ".local"*/;
    u8g2.drawStr(ALIGN_CENTER(Msg3.c_str()), yPos, Msg3.c_str());
}

// Tekent het hoofddashboard (fans, temps, etc)
void drawDashboard(const SensorData& d, time_t now)
{
    // --- 1. BOVENSTE BALK (Titel, NTP, WiFi) ---
    bool timeValid = (now > 1735689600);
    long ntpIcon = timeValid ? 57367 : 57368;
    long rssiIcon = map(WiFi.RSSI(), -90, -30, 57949, 57953);

    u8g2.setFont(u8g2_font_waffle_t_all); // Zorg dat dit font actief is
    u8g2.drawGlyph(0, 10, ntpIcon); // Y iets verlaagd naar 10 voor betere weergave
    u8g2.drawGlyph(14, 10, rssiIcon); // X iets meer ruimte gegeven

    u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(ALIGN_RIGHT("RADIATOR MONITOR"), 10, "RADIATOR MONITOR");
    u8g2.drawLine(0, 13, 128, 13);

    // --- 2. GROTE TEMPERATUUR ---
    u8g2.setFont(u8g2_font_helvB18_tf); // Groot font voor de hoofdwaarde
    char temp_buf[10];
    // Gebruik snprintf om de float mooi te formatteren (%.1f voor 1 decimaal)
    snprintf(temp_buf, sizeof(temp_buf), "%.1f%cC", d.tempC, 0xB0); // 0xB0 is het graden-symbool
    u8g2.drawStr(0, 45, temp_buf); // Y-coordinaat afhankelijk van je scherm hoogte

    // --- 3. FAN POWER BALK ---
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 56, "FAN POWER:");

    // Gebruik direct de waarde die we al berekend hebben (0-100)
    int pwmPercent = d.pwmPercentage; 
    
    // Teken het kader
    u8g2.drawFrame(0, 60, 128, 8);
    
    // Teken de vulling (map de 0-100% naar de 124 pixels breedte van het balkje)
    if (pwmPercent > 0) {
        int barWidth = map(pwmPercent, 0, 100, 0, 124);
        u8g2.drawBox(2, 62, barWidth, 4);
    }

    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 56, "FAN POWER:");

    char pwm_buf[10];
    snprintf(pwm_buf, sizeof(pwm_buf), "%d%%", pwmPercent);
    
    // Rechts uitlijnen van de tekst
    u8g2.setCursor(128 - u8g2.getStrWidth(pwm_buf), 56);
    u8g2.print(pwm_buf);

    // --- 4. FAN LIJST Temperatuur en RPM ---
    u8g2.setFont(u8g2_font_6x10_tf);
    int yPos = 85;
    int xPosCntr = u8g2.getDisplayWidth() / 2;

    // Fan 1
    char rpm_Buf[20];
    snprintf(temp_buf, sizeof(temp_buf), "%.1f%cC", d.tempCFan1, 0xB0);
    u8g2.drawStr(0, yPos, "Fan1:");
    u8g2.drawStr(xPosCntr - 10 - ALIGN_TXT_RIGHT(temp_buf), yPos, temp_buf);
    snprintf(rpm_Buf, sizeof(rpm_Buf), "%d RPM", d.fanRPM1);
    u8g2.drawStr(ALIGN_RIGHT(rpm_Buf), yPos, rpm_Buf);
    yPos += u8g2.getMaxCharHeight() + 2;
    // Fan 2
    snprintf(temp_buf, sizeof(temp_buf), "%.1f%cC", d.tempCFan2, 0xB0);
    u8g2.drawStr(0, yPos, "Fan2:");
    u8g2.drawStr(xPosCntr - 10 - ALIGN_TXT_RIGHT(temp_buf), yPos, temp_buf);
    snprintf(rpm_Buf, sizeof(rpm_Buf), "%d RPM", d.fanRPM2);
    u8g2.drawStr(ALIGN_RIGHT(rpm_Buf), yPos, rpm_Buf);
    yPos += u8g2.getMaxCharHeight() + 2;
    // Fan 3
    snprintf(temp_buf, sizeof(temp_buf), "%.1f%cC", d.tempCFan3, 0xB0);
    u8g2.drawStr(0, yPos, "Fan3:");
    u8g2.drawStr(xPosCntr - 10 - ALIGN_TXT_RIGHT(temp_buf), yPos, temp_buf);
    snprintf(rpm_Buf, sizeof(rpm_Buf), "%d RPM", d.fanRPM3);
    u8g2.drawStr(ALIGN_RIGHT(rpm_Buf), yPos, rpm_Buf);

    // --- 5. DATUM & TIJD ONDERAAN ---
    u8g2.setFont(u8g2_font_5x8_tf);
    // d.currentTimeStr moet je vullen in je main/network logic
    u8g2.drawStr(0, 128, d.currentTimeStr);
    u8g2.drawStr(ALIGN_RIGHT(d.currentDateStr), 128, d.currentDateStr);

    // Voeg hier de rest van je fans/data toe
}

// Helper functies voor OTA schermen (bovenaan in display_logic of lokaal)
void drawOTAScreen(const SensorData& d)
{
    u8g2.setFont(u8g2_font_6x10_tr); // Compact font
    u8g2.drawStr(10, 20, "SYSTEM UPDATE");
    u8g2.drawFrame(10, 30, 108, 10); // Kader voor voortgangsbalk
    u8g2.drawStr(10, 55, "Voorbereiden...");
}

void drawOTAProgress(const SensorData& d)
{
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(10, 20, "UPDATING...");

    // Voortgangsbalk tekenen
    u8g2.drawFrame(10, 30, 108, 10);
    uint8_t barWidth = map(d.otaProgress, 0, 100, 0, 104);
    u8g2.drawBox(12, 32, barWidth, 6);

    // Percentage tekst
    char buf[10];
    sprintf(buf, "%d%%", d.otaProgress);
    u8g2.drawStr(10, 55, buf);
}

// --- Hoofd Teken Functie (De State Machine) ---

void drawDisplay(struct tm* timeInfo, time_t now)
{
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
    u8g2.setContrast(d.displayContrast); // Pas het contrast aan volgens de gedeelde data
    u8g2.clearBuffer();
    if (d.huidigeFase != DASHBOARD) {
        u8g2.drawRFrame(1, 1, u8g2.getDisplayWidth() - 2, u8g2.getDisplayHeight() - 2, 6);
    }

    switch (d.huidigeFase) {
    case KOUDE_START:
        drawStartScreen(d);
        break;
    case INFO_SCHERM:
        drawInfoScreen(d);
        break;
    case OTA_START:
        drawOTAScreen(d);
        break;
    case OTA_PROGRESS:
        drawOTAProgress(d);
        break;
    case DASHBOARD:
        drawDashboard(d, now);
        break;
    }
    u8g2.sendBuffer();
}

// De display taak (apart op Core 0)
void displayTask(void* pvParameters)
{
    for (;;) {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        // if (now > 100000) {
        drawDisplay(timeInfo, now);
        // }
        vTaskDelay(pdMS_TO_TICKS(100)); // Teken 10x per seconde
    }
}

void setupDisplay()
{
    // Eventuele setup code indien nodig
}
