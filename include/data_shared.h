/*
 * (c)2026 R van Dorland - Gedeelde data structuur en
 * mutex voor thread-safe toegang
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Definieer de mogelijke FASES (de 'enum')
enum SysteemStatus {
    KOUDE_START,
    INFO_SCHERM,
    DASHBOARD,
    OTA_START,
    OTA_PROGRESS,
};

typedef struct {
    SysteemStatus huidigeFase;

    uint8_t otaProgress;

    unsigned long faseVervaltijd;

    // bool eersteStart;
    unsigned long startSchermVervaltijd;
    char sunriseStr[20];
    char sunsetStr[20];

    float tempC; // De radiator temperatuur
    float smoothedTemp;
    float tempCFan1, tempCFan2, tempCFan3;
    // voeg hier ook de rpms, fanDuty, currentDateStr, currentTimeStr toe

    int fanRPM1, fanRPM2, fanRPM3; // Compact en duidelijk
    int fanDuty;

    char currentDateStr[20]; // Ruime buffer
    char currentTimeStr[20]; // Ruime buffer
    char currentTime[9]; // "HH:MM:SS"
    char currentDate[11]; // "DD-MM-YYYY"
    uint8_t displayContrast; // Waarde 0-255

    long ntpIcon;
    long rssiIcon;
    bool timeValid;
} SensorData;

// Vertel elke module dat sensorData bestaat
extern SensorData sharedData;
extern SemaphoreHandle_t dataMutex;