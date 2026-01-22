/*
 * (c)2026 R van Dorland - Display Logica
 */

#pragma once
#include <U8g2lib.h>
#include <time.h>

// Functies
void setupDisplay();
void updateDisplay(struct tm* timeInfo, time_t now);

// Helpers voor uitlijning (optioneel, als je deze nog niet in helpers.h hebt)
#define GET_LCD_WIDTH() 128
#define GET_LCD_HEIGHT() 128
#define ALIGN_CENTER(t) ((GET_LCD_WIDTH() - u8g2.getStrWidth(t)) / 2)
#define ALIGN_V_CENTER() 64
