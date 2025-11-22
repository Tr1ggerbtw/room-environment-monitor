#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define MY_SSID CONFIG_ESP_WIFI_SSID
#define MY_PASS CONFIG_ESP_WIFI_PASSWORD
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define POST_URL  "http://192.168.0.101:3000/ping"

static const char *TAG = "Wifi-Station";
static const uint8_t MAX_RETRY = 10;
static uint8_t retry_num = 0; 

static EventGroupHandle_t s_wifi_event_group;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START){
    esp_wifi_connect();
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED){
        if(retry_num < MAX_RETRY){
            esp_wifi_connect();
            ++retry_num;
            ESP_LOGI(TAG, "Retrying to stabilize connection to Wi-Fi...");
        } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGI(TAG, "Connection have failed");
                } 
        } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
            ESP_LOGI(TAG, "Connection succesful");
            retry_num = 0; 
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
          }
}
         
// void wifi_init_sta()
// {

// }

void app_main(void) {

    // wifi_init_sta();
}