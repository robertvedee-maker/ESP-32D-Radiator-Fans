
/*
 * (c)2026 R van Dorland - Helpers (Strakke versie)
 */

#pragma once
#include "config.h"

// Gebruik 'inline' functies. Deze berekenen ALTIJD de actuele waarde
// op basis van de huidige font en het u8g2 object.
// We gebruiken u8g2_uint_t als return type om de compiler tevreden te houden

inline u8g2_uint_t GET_LCD_WIDTH()
{
    return u8g2.getDisplayWidth();
}

inline u8g2_uint_t GET_LCD_HEIGHT()
{
    return u8g2.getDisplayHeight();
}

// 1. Horizontale uitlijning
inline u8g2_uint_t ALIGN_CENTER(const char* t)
{
    return (u8g2_uint_t)((u8g2.getDisplayWidth() - u8g2.getUTF8Width(t)) / 2);
}

inline u8g2_uint_t ALIGN_RIGHT(const char* t)
{
    return (u8g2_uint_t)(u8g2.getDisplayWidth() - u8g2.getUTF8Width(t));
}

// 2. Verticale uitlijning
inline u8g2_uint_t GET_CHAR_HEIGHT()
{
    return (u8g2_uint_t)u8g2.getMaxCharHeight();
}

inline u8g2_uint_t ALIGN_V_CENTER()
{
    // Gebruik de interne u8g2 functies voor maximale compatibiliteit
    return (u8g2_uint_t)((u8g2.getDisplayHeight() + u8g2.getMaxCharHeight()) / 2);
}

// 3. Vaste posities
constexpr u8g2_uint_t ALIGN_LEFT = 0;
// We berekenen ALIGN_BOTTOM live via het object
inline u8g2_uint_t ALIGN_BOTTOM()
{
    return (u8g2_uint_t)u8g2.getDisplayHeight();
}

// --- Tijd en Datum Functies ---

// Zet double (18.5) om naar String ("18:30")
[[maybe_unused]] static String formatTime(double decimalTime)
{
    int h = (int)decimalTime;
    int m = (int)((decimalTime - h) * 60 + 0.5);
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", h, m);
    return String(buffer);
}

// Formatteert huidige tijd naar HH:MM:SS
[[maybe_unused]] static String formatCurrentTime(struct tm* timeinfo)
{
    char buffer[9];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return String(buffer);
}

[[maybe_unused]] void updateDateTimeStrings(struct tm* timeInfo) {
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        // Gebruik sharedData. voor de variabelen
        strftime(sharedData.currentTimeStr, sizeof(sharedData.currentTimeStr), "%H:%M:%S", timeInfo);
        strftime(sharedData.currentDateStr, sizeof(sharedData.currentDateStr), "%d-%m-%Y", timeInfo);
        xSemaphoreGive(dataMutex);
    }
}



// // Update de globale Strings voor tijd en datum
// [[maybe_unused]] inline void updateDateTimeStrings(struct tm* timeInfo)
// {
//     char buff[32];

//     // 1. Tijd opmaak
//     snprintf_P(buff, sizeof(buff), PSTR("%02d:%02d:%02d"),
//         timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
//     currentTimeStr = String(buff);

//     // 2. Datum opmaak: Zo.11 Jan 2026
//     const char* wday = (const char*)pgm_read_ptr(&wd_nl[timeInfo->tm_wday]);
//     const char* month = (const char*)pgm_read_ptr(&mo_nl[timeInfo->tm_mon]);

//     snprintf_P(buff, sizeof(buff), PSTR("%s.%02d %s %04d"),
//         wday, timeInfo->tm_mday, month, timeInfo->tm_year + 1900);
//     currentDateStr = String(buff);
// }
