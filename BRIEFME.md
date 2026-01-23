Project Context: ESP32 Radiator Fan Controller (D32 Dual Core)
Hardware: ESP32-WROOM-32D, SH1107 OLED (I2C), 3x Tacho (ISR + IRAM_ATTR), 1x PWM, DS18B20 OneWire.
Architectuur: Modulair (Split .h/.cpp).
Multicore Strategie:
Core 0: Systeemtaken (WiFi, OTA, NTP).
Core 1: displayTask (U8G2, 10Hz) en sensorTask (Orchestrator in helpers.cpp, 1Hz).
Data Handling: Gedeelde SensorData struct in data_shared.h, beveiligd met een dataMutex.
Belangrijkste Modules:
Modules: main, helpers (logic), display_logic, pwm_config (PWM/Tacho), onewire_config, daynight.
main.cpp: Setup & Task creation.
helpers.cpp: Orchestrator (sensorTask) die updateTemperatures, updateRPMs, manageBrightness en tijd-strings aanroept.
UI Logica: State-driven (enum SysteemStatus) met ondersteuning voor Dashboard, Info en OTA-visualisatie (progress bar).
display_logic.cpp: UI-logica; leest alleen via een lokale kopie uit de Mutex.
pwm_config.cpp: PWM-aansturing en Tacho-berekeningen (ISR's).
onewire_config.cpp: DS18B20 uitlezing.
daynight.cpp: Zonsopkomst/ondergang berekening en helderheidsbepaling.

Aangevulde Briefing (Versie 2.0 - Januari 2026)
Hardware: ESP32-WROOM-32D (D32), SH1107 OLED (I2C @ 400kHz), 3x Tacho (ISR + IRAM_ATTR), 1x PWM, DS18B20 OneWire bus.
Multicore Strategie:
Core 0 (Systeem): WiFi, ArduinoOTA, NTP.
Core 1 (Functioneel): displayTask (1Hz dashboard / 10Hz interactie) & sensorTask (Orchestrator).
Data Handling (Cruciaal):
Variabele: Centrale struct genaamd sharedData (voorheen sensorData).
Locatie: Gedeclareerd als extern in data_shared.h, fysiek gealloceerd in main.cpp.
Beveiliging: Toegang via dataMutex (Semaphore).
State-Machine UI:
Navigatie via enum SysteemStatus (KOUDE_START, INFO_SCHERM, DASHBOARD, OTA_START, OTA_PROGRESS).
Layout: Dynamisch kader (drawRFrame) voor systeemschermen, borderless voor dashboard.
Optimalisaties:
Non-blocking OneWire: setWaitForConversion(false) geactiveerd voor maximale Core 1 performance.
OTA Feedback: Visuele progress-bar gekoppeld aan de Core 0 callbacks.