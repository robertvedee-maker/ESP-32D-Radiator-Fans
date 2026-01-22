/*
 * (c)2026 R van Dorland - Display Logica
 */

#include "display_logic.h"
#include "config.h"
#include "onewire_config.h" // Voor tempC
#include "pwm_config.h"     // Voor rpms
#include "daynight.h"      // Voor sunrise/sunset strings
#include <WiFi.h>          // Voor WiFi.RSSI()

extern float smoothedTemp;
extern float TempCFan1, TempCFan2, TempCFan3;
extern DallasTemperature sensors;

// Gebruik het u8g2 object uit de main
extern U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2;

void setupDisplay() {
    u8g2.begin();
    u8g2.setContrast(10);
    
    // Optioneel: Startscherm
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    const char* msg = "Systeem Online";
    u8g2.drawStr(ALIGN_CENTER(msg), 64, msg);
    u8g2.sendBuffer();
    delay(1000);
}

void updateDisplay(struct tm* timeInfo, time_t now) {

    u8g2.clearBuffer();
    u8g2.enableUTF8Print();

    // --- 1. Bovenste balk: Iconen & Datum ---
    bool timeValid = (now > 1735689600);
    long ntpIcon = timeValid ? 57367 : 57368;
    long rssiIcon = map(WiFi.RSSI(), -90, -30, 57949, 57953);

    u8g2.setFont(u8g2_font_waffle_t_all); // Zorg dat dit font actief is
    u8g2.drawGlyph(0, 10, ntpIcon); // Y iets verlaagd naar 10 voor betere weergave
    u8g2.drawGlyph(14, 10, rssiIcon); // X iets meer ruimte gegeven

    // Header
    u8g2.setFont(u8g2_font_mozart_nbp_tf);
    u8g2.drawStr(32, 10, "RADIATOR MONITOR");
    u8g2.drawLine(0, 13, 128, 13);

    // Grote Temperatuur Weergave
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(0, 40);
    // u8g2.print(tempC, 1); // Toon 1 decimaal
    u8g2.print(smoothedTemp, 1); // Toon 1 decimaal
    u8g2.print("ºC"); // Dankzij enableUTF8Print()


    // PWM Balk midden
    int pwmPercent = map(fanDuty, 0, 1023, 0, 100);
    u8g2.drawFrame(0, 56, 128, 8);
    u8g2.drawBox(2, 58, map(pwmPercent, 0, 100, 0, 124), 4);
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 53, "FAN POWER:");
    u8g2.setCursor(110, 53);
    u8g2.print(pwmPercent);
    u8g2.print("%");

    // RPM Links
    u8g2.setFont(u8g2_font_mozart_nbp_tf);
    u8g2.setCursor(0, 80);
    u8g2.print("Fan 1:");
    u8g2.print(rpms[0]);
    u8g2.setCursor(0, 92);
    u8g2.print("Fan 2:");
    u8g2.print(rpms[1]);
    u8g2.setCursor(0, 104);
    u8g2.print("Fan 3:");
    u8g2.print(rpms[2]);

    TempCFan1 = sensors.getTempCByIndex(0); // tempC fan Air links
    TempCFan2 = sensors.getTempCByIndex(1); // tempC fan Middel
    TempCFan3 = sensors.getTempCByIndex(2); // tempC fan Air rechts

    // tempC fan Air rechts
    u8g2.setCursor(74, 80);
    u8g2.print(TempCFan1, 1);u8g2.print("ºC");
    u8g2.setCursor(74, 92);
    u8g2.print(TempCFan2, 1);u8g2.print("ºC");
    u8g2.setCursor(74, 104);
    u8g2.print(TempCFan3, 1);u8g2.print("ºC");

    // datum en tijd onderaan
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.setCursor(0, 126);
    u8g2.print(currentDateStr);
    u8g2.setCursor(90, 126);
    u8g2.print(currentTimeStr);



    u8g2.sendBuffer();
    // delay(200);

    // // 3. Output naar Seriële Monitor
    // Serial.print("NTC: ");
    // Serial.print(rawValue);
    // Serial.print(" | PWM: ");
    // Serial.print((fanDuty / 1023.0) * 100);
    // Serial.print("% | Fan Snelheid: ");
    // Serial.print(rpm);
    // Serial.println(" RPM");

    delay(5000); // Update elke 5 seconden
    u8g2.sendBuffer();





    // u8g2.clearBuffer();
    
    // // --- Voorbeeld van logica die je uit main.cpp knipt ---
    
    // // Kader
    // u8g2.drawRFrame(0, 0, 128, 128, 5);
    
    // // Klok
    // u8g2.setFont(u8g2_font_logisoso24_tf);
    // // (Hier komt jouw specifieke code voor currentTimeStr etc.)
    
    // // Temperaturen & Fans
    // u8g2.setFont(u8g2_font_6x10_tf);
    // u8g2.setCursor(10, 100);
    // u8g2.print("Temp: "); u8g2.print(tempC); u8g2.print(" C");
    
    // u8g2.setCursor(10, 115);
    // u8g2.print("Fan: "); u8g2.print(rpms[0]); u8g2.print(" RPM");

    // u8g2.sendBuffer();
}
