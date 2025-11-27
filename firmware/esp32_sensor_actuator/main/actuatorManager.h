#ifndef ACTUATOR_MANAGER_H
#define ACTUATOR_MANAGER_H
#include "esp_log.h"
#include "esp_check.h"

esp_err_t actuator_init();

esp_err_t fan_set_state(int state);

#endif