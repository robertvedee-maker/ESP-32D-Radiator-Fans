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

// Deze functie draait straks op Core 0
void TaskWorkerCore0(void* pvParameters)
{
    // Statische variabelen voor timing binnen deze task
    static unsigned long lastDisplayUpdate = 0;

    for (;;) { // Een oneindige lus (deze task stopt nooit)
        unsigned long currentMillis = millis();

        // Doe alle updates die elke seconde nodig zijn
        if (currentMillis - lastDisplayUpdate >= 1000) {
            lastDisplayUpdate = currentMillis;

            // Haal de tijd op
            time_t now = time(nullptr);
            struct tm* timeInfo = localtime(&now);

            // Alleen actie als tijd geldig is
            if (now > 100000) {
                updateTemperatures(); // Uit onewire_config.cpp
                updateRPMs(); // Uit pwm_config.cpp
                // updateDateTimeStrings(timeInfo); // Uit helpers.cpp
                manageBrightness(); // Uit daynight.cpp
                // updateDisplay(timeInfo, now); // Uit display_logic.cpp
            }
        }

        // Laat de CPU even ademen, voorkomt crashes
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup()
{
    delay(1000); // Wacht even voor stabiliteit

    Serial.begin(115200);

    dataMutex = xSemaphoreCreateMutex();
    if (dataMutex == NULL) {
        Serial.println("FATALE FOUT: Mutex kon niet worden aangemaakt!");
    }

    // Initialiseer de startfase hier, veilig na de mutex
    if (esp_reset_reason() == ESP_RST_POWERON) {
        sharedData.huidigeFase = KOUDE_START;
        sharedData.faseVervaltijd = millis() + 4000;
    } else {
        // Bij warme start: sla het IP-scherm over of toon het heel kort
        sharedData.huidigeFase = INFO_SCHERM; // Software reset
        sharedData.faseVervaltijd = millis() + 2000;
    }

    // Start de display-taak op Core 0
    xTaskCreatePinnedToCore(
        displayTask, // Functie die de taak uitvoert (in display_logic.cpp)
        "DisplayTask", // Naam van de taak
        8192, // Stack grootte (ruim voor u8g2)
        NULL, // Parameters
        2, // Prioriteit (hoger dan idle)
        NULL, // Taak handle (niet nodig tenzij je 'm wilt verwijderen)
        0 // PIN NAAR CORE 0
    );

    xTaskCreatePinnedToCore(
        sensorTask, // Functie naam
        "SensorTask", // Naam voor debuggen
        2048, // Stack size
        NULL, // Parameters
        2, // Prioriteit (bijv. 2)
        NULL, // Task handle
        0 // Op Core 0
    );

    xTaskCreatePinnedToCore(
        pwmTask, /* Functie naam */
        "RadiatorFansTask", /* Naam van de taak (max 16 karakters) */
        2048, /* Stack size in bytes */
        NULL, /* Parameter (geen) */
        1, /* Prioriteit (0 is laagst, 4 is hoogst) */
        NULL, /* Task handle (geen nodig) */
        0 /* Core ID (0 of 1) */
    );

        // Start de worker task op Core 0
    xTaskCreatePinnedToCore(
        TaskWorkerCore0, /* Functie naam */
        "DisplaySensorTask", /* Naam van de taak (max 16 karakters) */
        10000, /* Stack size in bytes */
        NULL, /* Parameter (geen) */
        1, /* Prioriteit (0 is laagst, 4 is hoogst) */
        NULL, /* Task handle (geen nodig) */
        0 /* Core ID (0 of 1) */
    );


    // Initialiseer Modules
    // setupPWM();

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    u8g2.begin();
    u8g2.setContrast(10);

    setupWiFi(SECRET_SSID, SECRET_PASS);
    setupOTA(DEVICE_MDNS_NAME);
    configTzTime(SECRET_TZ_INFO, SECRET_NTP_SERVER);

    fansOn();

    Serial.println("Systeem Modulair & Dual-Core Opgestart.");
}

void loop()
{
    // Dit blijft op Core 1 (WiFi core) draaien
    ArduinoOTA.handle();

    // Deze loop draait op Core 1 (Pro CPU)
    // Verzamel hier je data en werk de sharedData struct bij (met de mutex!)

    // Voorbeeld van veilig bijwerken van data:
    if (dataMutex != NULL) {
        if (xSemaphoreTake(dataMutex, (TickType_t)10) == pdTRUE) {
            // Update hier sharedData velden (bijv. sharedData.temp_radiator = lees_temp();)
            xSemaphoreGive(dataMutex); // Geef de mutex direct weer vrij
        }
    }

    // ... verder je network_logic_loop() of pwm_config_loop() ...

    // Tijd & Display logica
    unsigned long currentMillis = millis();
    static unsigned long lastDisplayUpdate = 0;

    if (currentMillis - lastDisplayUpdate >= 1000) {
        lastDisplayUpdate = currentMillis;
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);

        if (now > 100000) {
            // updateDateTimeStrings(timeInfo);
            manageBrightness();
            drawDisplay(timeInfo, now); // De nieuwe overzichtelijke aanroep
        }
    }

    delay(10);

    // De rest van de loop is nu leeg, want alles zit in TaskWorkerCore0

    // CRUCIAAL: Voorkom dat Core 1 te snel runt en crasht
    vTaskDelay(pdMS_TO_TICKS(10));
}

