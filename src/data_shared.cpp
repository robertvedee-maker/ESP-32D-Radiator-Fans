#include "data_shared.h"

// Hier worden ze echt aangemaakt (Definitie)
// Geef ze hier alleen een 'lege' startwaarde
SensorData sharedData; 
SemaphoreHandle_t dataMutex = NULL;

// // Hier wordt de ruimte in het geheugen echt gereserveerd (Definitie)
// SensorData sharedData = {
//     .eersteStart = true,
//     .sunriseStr = "--:--",
//     .sunsetStr = "--:--",
//     .tempC = 0.0,
//     .smoothedTemp = 0.0,
//     .tempCFan1 = 0.0,
//     .tempCFan2 = 0.0,
//     .tempCFan3 = 0.0,
//     .rpms = {0, 0, 0},
//     .fanDuty = 0,
//     .currentDateStr = "--. --:---:----",
//     .currentTimeStr = "--:--:--",
//     .ntpIcon = 0,
//     .rssiIcon = 0,
//     .timeValid = false
// };
// SemaphoreHandle_t dataMutex = NULL; 
