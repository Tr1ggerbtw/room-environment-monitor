#include <actuatorManager.h>
#include "driver/gpio.h"
#define LED_PIN GPIO_NUM_22
/*
for now just turning on, and turning off led using a simple GPIO.
todo: connect a relay module that can be used to control the fan voltage
*/
esp_err_t actuator_init()
{
    esp_err_t result = gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
    return result;
} 

esp_err_t fan_set_state(int state)
{
    esp_err_t result = gpio_set_level(LED_PIN, state);
    return result;
}