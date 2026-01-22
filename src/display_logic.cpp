/*
 * (c)2026 R van Dorland - Display Logica
 */

#include "display_logic.h"
#include "data_shared.h"

// Referentie naar het u8g2 object in main.cpp
extern U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2;

void setupDisplay() {
    // Hier kun je specifieke scherm-init zetten indien nodig
}

void drawDisplay(struct tm* timeInfo, time_t now) {
    u8g2.clearBuffer();
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 12, sharedData.currentTimeStr);
        u8g2.drawStr(0, 24, sharedData.sunriseStr);
        u8g2.drawStr(60, 24, sharedData.sunsetStr);
        xSemaphoreGive(dataMutex);
    }
    u8g2.sendBuffer();
}

// De taak die op Core 0 draait
void displayTask(void* pvParameters) {
    for (;;) {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);
        if (now > 100000) {
            drawDisplay(timeInfo, now);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Update elke seconde
    }
}
