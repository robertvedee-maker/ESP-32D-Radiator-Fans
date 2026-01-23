/*
 * (c)2026 R van Dorland - Display Logica
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Definieer de mogelijke FASES (de 'enum')
enum SysteemStatus {
    KOUDE_START,
    INFO_SCHERM,
    DASHBOARD
};

typedef struct {
    SysteemStatus huidigeFase;
    unsigned long faseVervaltijd;

    bool eersteStart;
    unsigned long startSchermVervaltijd;
    char sunriseStr[20];
    char sunsetStr[20];

    float tempC; // De radiator temperatuur
    float smoothedTemp;
    float tempCFan1, tempCFan2, tempCFan3;
    // voeg hier ook de rpms, fanDuty, currentDateStr, currentTimeStr toe

    int rpms[3];
    int fanDuty;
    char currentDateStr[20]; // Ruime buffer
    char currentTimeStr[20]; // Ruime buffer
    long ntpIcon;
    long rssiIcon;
    bool timeValid;
} SensorData;

extern SensorData sharedData;
extern SemaphoreHandle_t dataMutex;