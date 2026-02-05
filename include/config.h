/*
 * (c)2026 R van Dorland - Configuraties
 */

#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

// 2. Extern objecten (gedefinieerd in main.cpp)
typedef U8G2_SH1107_SEEED_128X128_F_HW_I2C DisplayType;
constexpr auto DISPLAY_ROTATION = U8G2_R1; // We gebruiken 'auto' zodat de compiler zelf het juiste u8g2-type kiest
constexpr int8_t DISPLAY_RESET_PIN = U8X8_PIN_NONE;

// 1.  Hardware pinnen
constexpr int I2C_SDA_PIN = 21;
constexpr int I2C_SCL_PIN = 22;

// Display start scherm duur in milliseconden
constexpr unsigned long START_SCHERM_DUUR = 5000; // 5 seconden in milliseconden

// Gebruik 6dB attenuatie voor 0-2.2V bereik (nauwkeuriger voor spanningsdeler 1/2)
// analogSetPinAttenuation(BATTERY_PIN, ADC_6db); in de void setup()
constexpr int BATTERY_PIN = 1; // ADC pin voor batterijmeting
constexpr int ADC_MAX = 4095; // Voor ESP32-C3 (12-bit ADC)
constexpr float VOLT_DR = 2.0f; // Verhouding van de spanningsdeler
constexpr float BAT_CAL = 2.03f; // Kalibratiefactor voor batterijmeting (afhankelijk van je spanningsdeler)
constexpr int BAT_MIN_VOLT = 330; // Hekwerk ondergrens
constexpr int BAT_MAX_VOLT = 420; // Hekwerk bovengrens
constexpr int BAT_GLYPH_LOW = 57922; // Waffle glyph leeg
constexpr int BAT_GLYPH_HIGH = 57931; // Waffle glyph vol

// 3. Globale variabelen (extern)
extern unsigned long lastBrightnessCheck;
extern const unsigned long brightnessInterval; // Alleen aankondigen

extern double sunrise_local;
extern double sunset_local;
extern String sunriseStr;
extern String sunsetStr;
extern String currentTimeStr;
extern String currentDateStr;

// Nederlandse namen voor weekdagen en maanden
static const char* const wd_nl[] PROGMEM = { "Zo", "Ma", "Di", "Wo", "Do", "Vr", "Za" };
static const char* const mo_nl[] PROGMEM = { "Jan", "Feb", "Mrt", "Apr", "Mei", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dec" };
