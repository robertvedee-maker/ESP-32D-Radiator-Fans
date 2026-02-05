Project Briefing: ESP32 Radiator Monitor v1.0
1. Systeem Architectuur
Controller: ESP32-WROOM-32D (Dual-Core).
Core-verdeling:
Core 0: Netwerk-taken (WiFi, OTA, NTP).
Core 1: Applicatie-logica (Sensoren, PWM-berekening en Display).
Communicatie: Real-time data-uitwisseling tussen cores via een Mutex-beveiligde struct (sharedData).
Framework: Arduino ESP32 Core v3.0 (PlatformIO).
2. Hardware & Beveiliging
Voeding: 13.5V externe adapter via een expansion board (ESP gevoed via 5V/VIN).
Temperatuur: 4x DS18B20 OneWire sensoren (3x Fans, 1x Radiator) op GPIO 18 met een 4.7k (of 2.2k) pull-up.
Fans: 3x 4-pins PWM-ventilatoren.
PWM-regeling: 25kHz frequentie op GPIO 13, 10-bit resolutie (0-1023).
Tacho/RPM: GPIO 25, 26, 27 met hardwarematige pull-up naar 3.3V en softwarematige debouncing (3-10ms).
Safety Fixes:
Vrijloopdiode (1N4007) over de fan-voedingslijn tegen inductiepieken.
Geplande upgrade: Opto-geïsoleerde MOSFET module voor galvanische scheiding van het vermogensdeel.
3. Software Logica
Temperatuur: Robuuste uitlezing met validatie (negeren van -127°C / 85°C uitschieters).
Fan-Curve: Progressieve regeling tussen 30°C (1200 RPM / Min PWM) en 55°C (Max RPM / 100% PWM).
Display: SH1107 OLED op I2C (GPIO 21/22) via U8G2, toont live temps, RPM's en een dynamisch "Fan Power" balkje.
RPM Filtering: Interrupt-gebaseerde telling met tijdsmeting om ruis op de Tacho-lijn te negeren.
4. Belangrijke Issues & Oplossingen (Lesson Learned)
Fysieke schade: Eerdere ESP32 defect geraakt door waarschijnlijk ompoling of inductiepiek; opgelost door nieuwe chip en toevoeging van beveiligingscomponenten.
Software crashes: TypeError in esptool door ontbrekende FlashID; opgelost door hardwarematige herverbinding van de flash-chip.
Ruis: 20.000 RPM metingen door interferentie; opgelost door verwijderen van te zware condensatoren (100nF) en implementatie van een softwarematige debounceTime.



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