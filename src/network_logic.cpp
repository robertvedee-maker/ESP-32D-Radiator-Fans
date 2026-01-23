/*
 * (c)2026 R van Dorland
 */

#include "network_logic.h"
#include "helpers.h"
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <esp_system.h>
#include <WiFi.h>


void toonNetwerkInfo() {
    esp_reset_reason_t reset_reason = esp_reset_reason();

    if (reset_reason == ESP_RST_POWERON || reset_reason == ESP_RST_SW) {
        u8g2.clearBuffer();
        u8g2.drawRFrame(0, 0, GET_LCD_WIDTH(), GET_LCD_HEIGHT(), 5);
        u8g2.setFont(u8g2_font_helvR08_tf);
        
        const char* title = "SYSTEEM START";
        u8g2.drawStr(ALIGN_CENTER(title), 15, title);

        u8g2.setCursor(12, 35);
        u8g2.print("IP:   " + WiFi.localIP().toString());
        u8g2.setCursor(12, 48);
        u8g2.print("mDNS: " + String(DEVICE_MDNS_NAME));

        u8g2.sendBuffer();
        delay(4000);
    }
}

void setupWiFi(const char* ssid, const char* password) {
    // WiFi.setSleep(false); 
    WiFi.begin(ssid, password);
    WiFi.setTxPower(WIFI_POWER_8_5dBm); 

    unsigned long startAttemptTime = millis();
    u8g2.setFont(u8g2_font_helvR08_tf);

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        const char* msg = "WiFi Verbinden";
        u8g2.clearBuffer();
        u8g2.drawRFrame(0, 0, GET_LCD_WIDTH(), GET_LCD_HEIGHT(), 5);
        u8g2.drawStr(ALIGN_CENTER(msg), ALIGN_V_CENTER(), msg);
        u8g2.sendBuffer();
        delay(1000);
    }
}

void setupOTA(const char* hostname) {
    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.onStart([]() {
        const char* msg = "OTA Update Start...";
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_helvR08_tf);
        u8g2.drawRFrame(0, 0, GET_LCD_WIDTH(), GET_LCD_HEIGHT(), 5);
        u8g2.drawStr(ALIGN_CENTER(msg), ALIGN_V_CENTER(), msg);
        u8g2.sendBuffer();
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        const char* msg = "Bezig met UPDATEN";
        u8g2.clearBuffer();
        u8g2.drawRFrame(0, 0, GET_LCD_WIDTH(), GET_LCD_HEIGHT(), 5);
        u8g2.drawStr(ALIGN_CENTER(msg), ALIGN_V_CENTER() - 10, msg);
        
        unsigned int width = (progress / (total / 100));
        u8g2.drawBox(14, ALIGN_V_CENTER() + 5, width, 5);
        u8g2.sendBuffer();
    });

    ArduinoOTA.begin();

}
