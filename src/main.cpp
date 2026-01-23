/*
 * (c)2026 R van Dorland
 */

#include "data_shared.h"
#include "display_logic.h" // Zorg dat deze bovenaan staat
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

#include "config.h"
#include "daynight.h"
#include "helpers.h"
#include "network_logic.h"
#include "onewire_config.h"
#include "pwm_config.h"
#include "secret.h"

DisplayType u8g2(DISPLAY_ROTATION, DISPLAY_RESET_PIN);

// void drawDisplay(struct tm* timeInfo, time_t now);
void displayTask(void* pvParameters);
void sensorTask(void* pvParameters);

void setup() {
    delay(1000); 
    Serial.begin(115200);

    // 1. Mutex als allereerste
    dataMutex = xSemaphoreCreateMutex();
    
    // 2. Hardware Initialisatie
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    u8g2.begin();
    setupPWM(); // Vergeet deze niet te un-commenten
    
    // 3. Status instellen
    if (esp_reset_reason() == ESP_RST_POWERON) {
        sharedData.huidigeFase = KOUDE_START;
        sharedData.faseVervaltijd = millis() + 2000;
    } else {
        sharedData.huidigeFase = INFO_SCHERM;
        sharedData.faseVervaltijd = millis() + 1000;
    }

    // 4. Netwerk & Tijd (Core 0 - De "Systeem" core)
    setupWiFi(SECRET_SSID, SECRET_PASS);
    setupOTA(DEVICE_MDNS_NAME);
    configTzTime(SECRET_TZ_INFO, SECRET_NTP_SERVER);

    // 5. Taken aanmaken
    
    // Display & UI (Core 1 - De "Applicatie" core)
    // We geven deze een hoge prioriteit zodat het scherm soepel ververst
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", 8192, NULL, 3, NULL, 1);

    // Sensoren & PWM berekeningen (Core 1)
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 2, NULL, 1);
    
    fansOn();
    Serial.println("Systeem Dual-Core Opgestart.");
}

void loop() {

    // Core 1 wordt ook gebruikt door loop(). 
    // Omdat we OTA gebruiken, houden we dit hier.
    ArduinoOTA.handle();
    
    // Optioneel: Update hier de systeem-tijd strings voor sharedData
    static unsigned long lastTimeUpdate = 0;
    if (millis() - lastTimeUpdate >= 1000) {
        lastTimeUpdate = millis();
        
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);

        if (now > 100000 && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50))) {
            // Update hier je sharedData.currentTime etc.
            // Zodat displayTask dit simpelweg kan uitlezen.
            xSemaphoreGive(dataMutex);
        }
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Cruciaal voor stabiliteit
}

