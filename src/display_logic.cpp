/*
 * (c)2026 R van Dorland - Display Logica
 */

#include "display_logic.h"
#include "data_shared.h" // Nu halen we alles hier vandaan!
// Verwijder hier de includes naar onewire_config.h, pwm_config.h, etc.

// extern U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2; // Blijft extern, want u8g2 is een hardware object
U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);


// Functie om de hoofd pagina te tekenen (leest data via mutex)
void drawMainPage(U8G2 *display) {
    display->clearBuffer();
    display->setFont(u8g2_font_6x10_tf);
    display->drawStr(0, 10, "Radiator Monitor");

    // Lees data veilig uit de gedeelde struct
    if( dataMutex != NULL && xSemaphoreTake( dataMutex, ( TickType_t ) 10 ) == pdTRUE ) {
        char tempStr[10];
        sprintf(tempStr, "%.1f C", sharedData.smoothedTemp);
        display->setFont(u8g2_font_helvR18_tf);
        display->drawStr(0, 40, tempStr);
        // ... teken de rest van je waarden ...

        xSemaphoreGive( dataMutex ); // Vrijgeven!
    } else {
        display->drawStr(0, 40, "Geen Data");
    }

    display->sendBuffer();
}


// Hier komt de displayCpuStats() functie van eerder, aangepast
// ... (code weggelaten voor beknoptheid, maar plaats hem hier)

// De hoofdtaak voor het display, draait op Core 0
void displayTask(void *pvParameters) {
    // Initialiseer je display hier, NA setup() in main.cpp is gerund
    // u8g2.begin();

    // Enum en touch logica van eerdere suggestie hier implementeren
    // Screen currentScreen = MAIN_PAGE;
    // ... touch logic variables ...

    for (;;) {
        // handleTouch(); // Controleer de touch input

        // if (currentScreen == MAIN_PAGE) {
        //     drawMainPage(&u8g2);
        // } else {
        //     displayCpuStats(&u8g2); 
        // }

        // Deze delay is CRUCIAAL voor Core 0, anders triggert de Watchdog
        vTaskDelay(pdMS_TO_TICKS(50)); // Update frequentie van 20Hz
    }
}






/*
// Functie om de hoofd pagina te tekenen (leest data via mutex)
void drawMainPage(U8G2* display)
{
    display->clearBuffer();
    display->enableUTF8Print();

    // LEES DATA VEILIG VIA MUTEX
    if (dataMutex != NULL && xSemaphoreTake(dataMutex, (TickType_t)50) == pdTRUE) {
        // --- Hier komt NU bijna 1-op-1 je bestaande updateDisplay code ---

        // --- 1. Bovenste balk: Iconen & Datum (gebruik sharedData.) ---
        display->setFont(u8g2_font_waffle_t_all);
        display->drawGlyph(0, 10, sharedData.ntpIcon);
        display->drawGlyph(14, 10, sharedData.rssiIcon);

        // Header
        display->setFont(u8g2_font_mozart_nbp_tf);
        display->drawStr(32, 10, "RADIATOR MONITOR");
        display->drawLine(0, 13, 128, 13);

        // Grote Temperatuur Weergave
        display->setFont(u8g2_font_helvB18_tf);
        display->setCursor(0, 40);
        display->print(sharedData.smoothedTemp, 1); // Gebruik sharedData
        display->print("ºC");

        // PWM Balk midden (gebruik sharedData.fanDuty)
        int pwmPercent = map(sharedData.fanDuty, 0, 1023, 0, 100);
        // ... (rest van de tekenlogica blijft hetzelfde) ...

        // RPM Links (gebruik sharedData.rpms)
        // ...

        // tempC fan Air rechts (gebruik sharedData.tempCFan1/2/3)
        // Let op: Verwijder de sensors.getTempCByIndex() oproepen hier!
        // Die horen thuis in je onewire_config.cpp file op Core 1.

        // datum en tijd onderaan (gebruik sharedData.currentDateStr/TimeStr)
        // ...

        // -------------------------------------------------------------

        xSemaphoreGive(dataMutex); // Geef de mutex direct weer vrij
    } else {
        display->drawStr(0, 64, "Data Locked!");
    }

    display->sendBuffer();
    // Verwijder de delay(5000) hier! Die hoort in de displayTask loop.
}

// Hier komt je displayTask(void* pvParam) functie die de main loop vervangt
// en de paginanavigatie (MainPage vs StatsPage) afhandelt.

/*

#include "display_logic.h"
#include "data_share.h"
#include "config.h"
#include "onewire_config.h" // Voor tempC
#include "pwm_config.h"     // Voor rpms
#include "daynight.h"      // Voor sunrise/sunset strings
#include <WiFi.h>          // Voor WiFi.RSSI()

extern float smoothedTemp;
extern float TempCFan1, TempCFan2, TempCFan3;
extern DallasTemperature sensors;

// Gebruik het u8g2 object uit de main
extern U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2;


// Functie om de hoofd pagina te tekenen (leest data via mutex)
void drawMainPage(U8G2 *display) {
    display->clearBuffer();
    display->setFont(u8g2_font_6x10_tf);
    display->drawStr(0, 10, "Radiator Monitor");

    // Lees data veilig uit de gedeelde struct
    if( dataMutex != NULL && xSemaphoreTake( dataMutex, ( TickType_t ) 10 ) == pdTRUE ) {
        char tempStr[10];
        sprintf(tempStr, "%.1f C", sharedData.temp_radiator);
        display->setFont(u8g2_font_helvR18_tf);
        display->drawStr(0, 40, tempStr);
        // ... teken de rest van je waarden ...

        xSemaphoreGive( dataMutex ); // Vrijgeven!
    } else {
        display->drawStr(0, 40, "Geen Data");
    }

    display->sendBuffer();
}

// Hier komt de displayCpuStats() functie van eerder, aangepast
// ... (code weggelaten voor beknoptheid, maar plaats hem hier)

// De hoofdtaak voor het display, draait op Core 0
void displayTask(void *pvParameters) {
    // Initialiseer je display hier, NA setup() in main.cpp is gerund
    // u8g2.begin();

    // Enum en touch logica van eerdere suggestie hier implementeren
    // Screen currentScreen = MAIN_PAGE;
    // ... touch logic variables ...

    for (;;) {
        // handleTouch(); // Controleer de touch input

        // if (currentScreen == MAIN_PAGE) {
        //     drawMainPage(&u8g2);
        // } else {
        //     displayCpuStats(&u8g2);
        // }

        // Deze delay is CRUCIAAL voor Core 0, anders triggert de Watchdog
        vTaskDelay(pdMS_TO_TICKS(50)); // Update frequentie van 20Hz
    }
}




void setupDisplay() {
    u8g2.begin();
    u8g2.setContrast(10);

    // Optioneel: Startscherm
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    const char* msg = "Systeem Online";
    int x = (128 - u8g2.getStrWidth(msg)) / 2;
    u8g2.drawStr(x, 64, msg);
    u8g2.sendBuffer();
    delay(1000);
}

void updateDisplay(struct tm* timeInfo, time_t now) {

    u8g2.clearBuffer();
    u8g2.enableUTF8Print();

    // --- 1. Bovenste balk: Iconen & Datum ---
    bool timeValid = (now > 1735689600);
    long ntpIcon = timeValid ? 57367 : 57368;
    long rssiIcon = map(WiFi.RSSI(), -90, -30, 57949, 57953);

    u8g2.setFont(u8g2_font_waffle_t_all); // Zorg dat dit font actief is
    u8g2.drawGlyph(0, 10, ntpIcon); // Y iets verlaagd naar 10 voor betere weergave
    u8g2.drawGlyph(14, 10, rssiIcon); // X iets meer ruimte gegeven

    // Header
    u8g2.setFont(u8g2_font_mozart_nbp_tf);
    u8g2.drawStr(32, 10, "RADIATOR MONITOR");
    u8g2.drawLine(0, 13, 128, 13);

    // Grote Temperatuur Weergave
    u8g2.setFont(u8g2_font_helvB18_tf);
    u8g2.setCursor(0, 40);
    // u8g2.print(tempC, 1); // Toon 1 decimaal
    u8g2.print(smoothedTemp, 1); // Toon 1 decimaal
    u8g2.print("ºC"); // Dankzij enableUTF8Print()


    // PWM Balk midden
    int pwmPercent = map(fanDuty, 0, 1023, 0, 100);
    u8g2.drawFrame(0, 56, 128, 8);
    u8g2.drawBox(2, 58, map(pwmPercent, 0, 100, 0, 124), 4);
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 53, "FAN POWER:");
    u8g2.setCursor(110, 53);
    u8g2.print(pwmPercent);
    u8g2.print("%");

    // RPM Links
    u8g2.setFont(u8g2_font_mozart_nbp_tf);
    u8g2.setCursor(0, 80);
    u8g2.print("Fan 1:");
    u8g2.print(rpms[0]);
    u8g2.setCursor(0, 92);
    u8g2.print("Fan 2:");
    u8g2.print(rpms[1]);
    u8g2.setCursor(0, 104);
    u8g2.print("Fan 3:");
    u8g2.print(rpms[2]);

    TempCFan1 = sensors.getTempCByIndex(0); // tempC fan Air links
    TempCFan2 = sensors.getTempCByIndex(1); // tempC fan Middel
    TempCFan3 = sensors.getTempCByIndex(2); // tempC fan Air rechts

    // tempC fan Air rechts
    u8g2.setCursor(74, 80);
    u8g2.print(TempCFan1, 1);u8g2.print("ºC");
    u8g2.setCursor(74, 92);
    u8g2.print(TempCFan2, 1);u8g2.print("ºC");
    u8g2.setCursor(74, 104);
    u8g2.print(TempCFan3, 1);u8g2.print("ºC");

    // datum en tijd onderaan
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.setCursor(0, 126);
    u8g2.print(currentDateStr);
    u8g2.setCursor(90, 126);
    u8g2.print(currentTimeStr);



    u8g2.sendBuffer();
    // delay(200);

    // // 3. Output naar Seriële Monitor
    // Serial.print("NTC: ");
    // Serial.print(rawValue);
    // Serial.print(" | PWM: ");
    // Serial.print((fanDuty / 1023.0) * 100);
    // Serial.print("% | Fan Snelheid: ");
    // Serial.print(rpm);
    // Serial.println(" RPM");

    delay(5000); // Update elke 5 seconden
    u8g2.sendBuffer();





    // u8g2.clearBuffer();

    // // --- Voorbeeld van logica die je uit main.cpp knipt ---

    // // Kader
    // u8g2.drawRFrame(0, 0, 128, 128, 5);

    // // Klok
    // u8g2.setFont(u8g2_font_logisoso24_tf);
    // // (Hier komt jouw specifieke code voor currentTimeStr etc.)

    // // Temperaturen & Fans
    // u8g2.setFont(u8g2_font_6x10_tf);
    // u8g2.setCursor(10, 100);
    // u8g2.print("Temp: "); u8g2.print(tempC); u8g2.print(" C");

    // u8g2.setCursor(10, 115);
    // u8g2.print("Fan: "); u8g2.print(rpms[0]); u8g2.print(" RPM");

    // u8g2.sendBuffer();
}
*/