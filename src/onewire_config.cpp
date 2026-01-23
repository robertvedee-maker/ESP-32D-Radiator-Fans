/*
 * (c)2026 R van Dorland - OneWire Configuraties voor DS18B20 sensoren
 */

#include "onewire_config.h"
#include "data_shared.h"

#define ONE_WIRE_BUS 18

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Hardware adressen
DeviceAddress addrFan1 = { 0x28, 0x7C, 0x6E, 0x6F, 0x00, 0x00, 0x00, 0x42 };
DeviceAddress addrFan2 = { 0x28, 0xEC, 0x54, 0x76, 0x00, 0x00, 0x00, 0x61 };
DeviceAddress addrFan3 = { 0x28, 0xD2, 0x68, 0x71, 0x00, 0x00, 0x00, 0xBC };
DeviceAddress addrRad = { 0x28, 0x99, 0x76, 0x6F, 0x00, 0x00, 0x00, 0xC2 }; // Let op: C2 of BC?

// DE FUNCTIE IMPLEMENTATIE (De 'Body')
void updateTemperatures()
{
    sensors.requestTemperatures(); // Vraag alle sensoren om data

    float t1 = sensors.getTempC(addrFan1);
    float t2 = sensors.getTempC(addrFan2);
    float t3 = sensors.getTempC(addrFan3);
    float tR = sensors.getTempC(addrRad);

    // Schrijf de waarden veilig weg naar de Mutex
    if (dataMutex != NULL && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        sharedData.tempCFan1 = t1;
        sharedData.tempCFan2 = t2;
        sharedData.tempCFan3 = t3;
        sharedData.tempC = tR; // Radiator temp
        xSemaphoreGive(dataMutex);
    }
}
