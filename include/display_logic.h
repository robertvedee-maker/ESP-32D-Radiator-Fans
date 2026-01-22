/*
 * (c)2026 R van Dorland - Display Logica 
 */

#pragma once
#include <U8g2lib.h>
#include <time.h>

// 1. Gebruik veiligere constexpr voor afmetingen
constexpr int LCD_WIDTH = 128;
constexpr int LCD_HEIGHT = 128;

// 2. Gebruik inline functies in plaats van complexe #define macros
void setupDisplay();
void updateDisplay(struct tm* timeInfo, time_t now);

// Helpers voor uitlijning
extern U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2;

// inline int ALIGN_CENTER(const char* t) {
//     return (LCD_WIDTH - u8g2.getStrWidth(t)) / 2;
// }

// inline int ALIGN_V_CENTER() {
//     return LCD_HEIGHT / 2;
// }
