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
         
void wifi_init_sta()
{
s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifiInitCfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifiInitCfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = MY_SSID,
            .password = MY_PASS,
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if(bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connected to ap");
    }   else if (bits & WIFI_FAIL_BIT) 
        {
            ESP_LOGI(TAG, "Failed to connect to AP");
        } else {
            ESP_LOGI(TAG, "Unexpected event");
        }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
}