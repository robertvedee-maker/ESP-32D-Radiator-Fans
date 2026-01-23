/*
 * (c)2026 R van Dorland
 */

 #include "helpers.h"
#include "data_shared.h"
#include "config.h"
#include "daynight.h"

void manageBrightness()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    time_t now = time(nullptr);
    double transit, sunrise, sunset;

    // 1. Bereken de zonsopkomst en -ondergang (UTC)
    calcSunriseSunset(now, SECRET_LAT, SECRET_LON, transit, sunrise, sunset);

    // 2. Bepaal de lokale offset
    double utcOffset = (timeinfo.tm_isdst > 0) ? 2.0 : 1.0;
    double sunrise_local = sunrise + utcOffset;
    double sunset_local = sunset + utcOffset;

    // 3. Bepaal het gewenste contrast (zonder het display direct aan te raken)
    double currentHour = timeinfo.tm_hour + (timeinfo.tm_min / 60.0);
    uint8_t targetContrast;
    
    if (currentHour > sunrise_local && currentHour < sunset_local) {
        targetContrast = 100; // Dag-stand
    } else {
        targetContrast = 5;   // Nacht-stand
    }

    // 4. ALLES in één keer veilig naar sharedData schrijven
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        
        // Schrijf de zonsopkomst/ondergang strings
        snprintf(sharedData.sunriseStr, sizeof(sharedData.sunriseStr), "%s", formatTime(sunrise_local).c_str());
        snprintf(sharedData.sunsetStr, sizeof(sharedData.sunsetStr), "%s", formatTime(sunset_local).c_str());
        
        // Schrijf de huidige tijd string
        snprintf(sharedData.currentTime, sizeof(sharedData.currentTime), "%02d:%02d:%02d", 
                 timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
        // Schrijf het contrast niveau weg (de displayTask leest dit uit!)
        sharedData.displayContrast = targetContrast;

        xSemaphoreGive(dataMutex);
    }
}

