#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H
#include "esp_err.h"

typedef struct
{
    float temperature;
    float humidity;
} SensorData_t;

esp_err_t readData(SensorData_t *data);

#endif