/*
 * (c)2026 R van Dorland
 */

#include "daynight.h"
#include "helpers.h"
#include "data_shared.h"
#include "config.h"

// Het u8g2 object wordt extern aangeroepen vanuit main
extern U8G2_SH1107_SEEED_128X128_F_HW_I2C u8g2;

void manageBrightness()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    time_t now = time(nullptr);
    double transit, sunrise, sunset;

    // 1. Bereken de tijden (UTC)
    // De variabelen latitude en longitude moeten ergens gedefinieerd zijn (bijv. in config.h)
    calcSunriseSunset(now, SECRET_LAT, SECRET_LON, transit, sunrise, sunset);

    // 2. Bepaal de lokale offset
    double utcOffset = (timeinfo.tm_isdst > 0) ? 2.0 : 1.0;
    double sunrise_local = sunrise + utcOffset;
    double sunset_local = sunset + utcOffset;

    // 3. EENMALIG de data veilig wegschrijven naar de 'verzameltank'
    if (xSemaphoreTake(dataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        snprintf(sharedData.sunriseStr, sizeof(sharedData.sunriseStr), "%s", formatTime(sunrise_local).c_str());
        snprintf(sharedData.sunsetStr, sizeof(sharedData.sunsetStr), "%s", formatTime(sunset_local).c_str());
        xSemaphoreGive(dataMutex);
    }

    // 4. Contrast regelen
    double currentHour = timeinfo.tm_hour + (timeinfo.tm_min / 60.0);
    if (currentHour > sunrise_local && currentHour < sunset_local) {
        u8g2.setContrast(100); 
    } else {
        u8g2.setContrast(5); 
    }
}
