#include "data_shared.h"

// Hier wordt de ruimte in het geheugen echt gereserveerd (Definitie)
SensorData sharedData; 
SemaphoreHandle_t dataMutex = NULL; 
