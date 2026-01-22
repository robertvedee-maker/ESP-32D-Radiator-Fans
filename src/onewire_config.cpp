/*
 * (c)2026 R van Dorland
 */

#include "onewire_config.h"

// De pin-definitie (indien nog niet in .h)
const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

// 1. De unieke hardware adressen (geconverteerd van jouw lijst)
DeviceAddress addrFan1 = { 0x28, 0x7C, 0x6E, 0x6F, 0x00, 0x00, 0x00, 0x42 };
DeviceAddress addrFan2 = { 0x28, 0xEC, 0x54, 0x76, 0x00, 0x00, 0x00, 0x61 };
DeviceAddress addrFan3 = { 0x28, 0xD2, 0x68, 0x71, 0x00, 0x00, 0x00, 0xBC };
DeviceAddress addrRad  = { 0x28, 0x99, 0x76, 0x6F, 0x00, 0x00, 0x00, 0xC2 };

// 2. De globale variabelen voor de temperaturen
float TempCFan1 = 0.0;
float TempCFan2 = 0.0;
float TempCFan3 = 0.0;
float tempC     = 0.0; // Dit is radTempC (je hoofdvariabele)

void setupOneWire() {
    sensors.begin();
    // Optioneel: zet de resolutie op 10-bit (sneller dan standaard 12-bit)
    sensors.setResolution(10);
}

void updateTemperatures() {
    sensors.requestTemperatures();

    // Haal waarden op en check of de sensor nog verbonden is
    float t1 = sensors.getTempC(addrFan1);
    float t2 = sensors.getTempC(addrFan2);
    float t3 = sensors.getTempC(addrFan3);
    float tr = sensors.getTempC(addrRad);

    if (t1 != DEVICE_DISCONNECTED_C) TempCFan1 = t1;
    if (t2 != DEVICE_DISCONNECTED_C) TempCFan2 = t2;
    if (t3 != DEVICE_DISCONNECTED_C) TempCFan3 = t3;
    if (tr != DEVICE_DISCONNECTED_C) tempC = tr;

    // Debugging naar Serial
    /*
    Serial.printf("Rad: %.1f | F1: %.1f | F2: %.1f | F3: %.1f\n", tempC, TempCFan1, TempCFan2, TempCFan3);
    */
}

