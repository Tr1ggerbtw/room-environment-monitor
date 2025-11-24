#include "sensorManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h"

#define SENSOR_GPIO GPIO_NUM_4

const dht_sensor_type_t version = DHT_TYPE_DHT11;

esp_err_t readData(SensorData_t *data)
{
    esp_err_t result = dht_read_float_data(version, SENSOR_GPIO, &data->humidity, &data->temperature);

    if(result == ESP_OK)
    {
        return ESP_OK;
    } else{
        ESP_LOGE("Sensor", "Error of reading data from sensor: %s", esp_err_to_name(result));
        return result;
    }
}
