/*
 * (c)2026 R van Dorland
 */

#include "data_shared.h"
#include "onewire_config.h"
#include <DallasTemperature.h>
#include <OneWire.h>

// Defineer de OneWire bus pin (vervang PIN_NUMMER door jouw GPIO)
#define ONE_WIRE_BUS 18 // Bijvoorbeeld GPIO 15

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// 1. De unieke hardware adressen (geconverteerd van jouw lijst)
DeviceAddress addrFan1 = { 0x28, 0x7C, 0x6E, 0x6F, 0x00, 0x00, 0x00, 0x42 };
DeviceAddress addrFan2 = { 0x28, 0xEC, 0x54, 0x76, 0x00, 0x00, 0x00, 0x61 };
DeviceAddress addrFan3 = { 0x28, 0xD2, 0x68, 0x71, 0x00, 0x00, 0x00, 0xBC };
DeviceAddress addrRad = { 0x28, 0x99, 0x76, 0x6F, 0x00, 0x00, 0x00, 0xC2 };

void sensorTask(void* pvParameters)
{
    sensors.begin(); // Start de Dallas library eenmalig

    for (;;) {
        // Vraag nieuwe metingen aan de sensoren
        sensors.requestTemperatures();

        // Pak de mutex vast om veilig naar sharedData te schrijven
        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
            // Lees de waarden en stop ze in de gedeelde struct
            sharedData.tempC = sensors.getTempC(addrRad);
            sharedData.tempCFan1 = sensors.getTempC(addrFan1);
            sharedData.tempCFan2 = sensors.getTempC(addrFan2);
            sharedData.tempCFan3 = sensors.getTempC(addrFan3);

            // Geef de mutex weer vrij zodat de displayTask kan lezen
            xSemaphoreGive(dataMutex);
        }

        // Wacht 5 seconden voordat we opnieuw meten
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

