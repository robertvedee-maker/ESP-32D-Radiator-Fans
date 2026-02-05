/*
 * (c)2026 R van Dorland - Helper functies
 */

#include "helpers.h"
#include "data_shared.h"
#include "onewire_config.h"
#include "pwm_config.h"
#include "daynight.h"

// De logica van de functies
String formatTime(double decimalTime) {
    int h = (int)decimalTime;
    int m = (int)((decimalTime - h) * 60 + 0.5);
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", h, m);
    return String(buffer);
}

void updateDateTimeStrings(struct tm* timeInfo) {
    if (dataMutex != NULL && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        strftime(sharedData.currentTimeStr, sizeof(sharedData.currentTimeStr), "%H:%M:%S", timeInfo);
        strftime(sharedData.currentDateStr, sizeof(sharedData.currentDateStr), "%d-%m-%Y", timeInfo);
        xSemaphoreGive(dataMutex);
    }
}

void sensorTask(void* pvParameters) {
    for (;;) {
        time_t now = time(nullptr);
        struct tm* timeInfo = localtime(&now);

        updateDateTimeStrings(timeInfo);
        updateTemperatures(); // Deze staat in onewire_config.cpp
        updateRPMs();         // Deze staat in pwm_config.cpp
        setFanSpeed();      // Deze staat in pwm_config.cpp
        manageBrightness();   // Deze staat in daynight.cpp

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}







// #include "helpers.h"
// #include "data_shared.h"
// #include "onewire_config.h" // Voor updateTemperatures
// #include "pwm_config.h"     // Voor updateRPMs
// #include "daynight.h"       // Voor manageBrightness

// // ... formatTime en updateDateTimeStrings ...

// // DIT IS DE CENTRALE TASK
// void sensorTask(void* pvParameters) {
//     for (;;) {
//         // 1. Haal de tijd op voor de tijd-strings
//         time_t now = time(nullptr);
//         struct tm* timeInfo = localtime(&now);

//         // 2. Update alle data via hun eigen .cpp logica
//         updateDateTimeStrings(timeInfo); // helpers.cpp
//         updateTemperatures();            // onewire_config.cpp
//         updateRPMs();                    // pwm_config.cpp
//         manageBrightness();              // daynight.cpp

//         // 3. Wacht 1 seconde voor de volgende ronde
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }
