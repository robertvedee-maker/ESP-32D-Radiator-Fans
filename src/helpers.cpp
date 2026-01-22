#include "helpers.h"
#include "data_shared.h" // Cruciaal voor sharedData en dataMutex

String formatTime(double decimalTime) {
    int h = (int)decimalTime;
    int m = (int)((decimalTime - h) * 60 + 0.5);
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", h, m);
    return String(buffer);
}

void updateDateTimeStrings(struct tm* timeInfo) {
    if (dataMutex != NULL) {
        if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            strftime(sharedData.currentTimeStr, sizeof(sharedData.currentTimeStr), "%H:%M:%S", timeInfo);
            strftime(sharedData.currentDateStr, sizeof(sharedData.currentDateStr), "%d-%m-%Y", timeInfo);
            xSemaphoreGive(dataMutex);
        }
    }
}
