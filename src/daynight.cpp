/*
 * (c)2026 R van Dorland
 */

#include "daynight.h"
#include "data_shared.h" // Cruciaal voor toegang tot sharedData en dataMutex
#include "helpers.h"

double latitude = SECRET_LAT;
double longitude = SECRET_LON;

void manageBrightness()
{
    // ... bestaande berekeningen voor sunrise_local ...
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return;

    time_t now = time(nullptr);
    double transit, sunrise, sunset;

    // 1. Bereken de tijden (deze komen altijd terug in UTC)
    calcSunriseSunset(now, latitude, longitude, transit, sunrise, sunset);

    // 2. Bepaal de lokale offset (Winter = 1.0, Zomer = 2.0)
    double utcOffset = (timeinfo.tm_isdst > 0) ? 2.0 : 1.0;

    // 3. Tel de offset handmatig op bij de resultaten
    double sunrise_local = sunrise + utcOffset;
    double sunset_local = sunset + utcOffset;

    // 4. Formatteer de LOKALE tijden voor het display
    sunriseStr = formatTime(sunrise_local);
    sunsetStr = formatTime(sunset_local);

    // 5. Gebruik de lokale tijden voor de contrast-regeling
    double currentHour = timeinfo.tm_hour + (timeinfo.tm_min / 60.0);

    // Pas het contrast aan op basis van de zon
    if (currentHour > sunrise_local && currentHour < sunset_local) {
        u8g2.setContrast(100); // Overdag fel
    } else {
        u8g2.setContrast(5); // 's Nachts gedimd
    }

    // Schrijf de resultaten veilig naar de gedeelde struct
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Gebruik snprintf voor char-arrays in de struct
        snprintf(sharedData.sunriseStr, sizeof(sharedData.sunriseStr), "%s", formatTime(sunrise_local).c_str());
        snprintf(sharedData.sunsetStr, sizeof(sharedData.sunsetStr), "%s", formatTime(sunset_local).c_str());
        xSemaphoreGive(dataMutex);
    }

    // Gebruik het u8g2 object via de 'extern' referentie
    u8g2.setContrast(currentHour > sunrise_local && currentHour < sunset_local ? 100 : 5);

    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        // KOPIEER NAAR DE STRUCT (De nieuwe plek)
        snprintf(sharedData.sunriseStr, sizeof(sharedData.sunriseStr), "%s", formatTime(sunrise_local).c_str());
        snprintf(sharedData.sunsetStr, sizeof(sharedData.sunsetStr), "%s", formatTime(sunset_local).c_str());
        xSemaphoreGive(dataMutex);
    }
}
