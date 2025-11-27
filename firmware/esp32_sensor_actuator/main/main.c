#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "esp_http_server.h"
#include "sensorManager.h"
#include "actuatorManager.h"

#define MY_SSID CONFIG_ESP_WIFI_SSID
#define MY_PASS CONFIG_ESP_WIFI_PASSWORD
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define POST_URL "http://192.168.0.102:3000/ping"
#define BUF_LEN 1
#define periodTime 25000


static const char *WIFI_TAG = "Wifi-Station";
static const char *HCLIENT_TAG = "http-client";
static const char *HSERVER_TAG = "http-server";

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
            ESP_LOGI(WIFI_TAG, "Retrying to stabilize connection to Wi-Fi...");
        } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGI(WIFI_TAG, "Connection have failed");
                } 
        } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP){
            ESP_LOGI(WIFI_TAG, "Connection succesful");
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
        ESP_LOGI(WIFI_TAG, "Connected to ap");
    }   else if (bits & WIFI_FAIL_BIT) 
        {
            ESP_LOGI(WIFI_TAG, "Failed to connect to AP");
        } else {
            ESP_LOGI(WIFI_TAG, "Unexpected event");
        }
}

esp_err_t uri_post_handler(httpd_req_t *req)
{
    char content[BUF_LEN];

    size_t recv_size = req->content_len;

    if (recv_size != 1) 
    {
    ESP_LOGE(HSERVER_TAG, "Payload too long: %u bytes received", recv_size);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Expected 1 byte command");
    return ESP_FAIL;
    }
    
    httpd_req_recv(req, content, recv_size);
    
    esp_err_t result;

    if(content[0] == '0')
    {
        result = fan_set_state(content[0] - '0');
            if(result != ESP_OK)
            {
                const char failResp[] = "An error occured..";
                ESP_LOGE(HSERVER_TAG, "An error occured: %s", esp_err_to_name(result));
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, failResp);
                return result;
            }
        ESP_LOGI(HSERVER_TAG, "Turning off fan");
    }
    else if(content[0] == '1')
    {
        result = fan_set_state(content[0] - '0');
            if(result != ESP_OK)
            {
                const char failResp[] = "An error occured..";
                ESP_LOGE(HSERVER_TAG, "An error occured: %s", esp_err_to_name(result));
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, failResp);
                return result;  
            }
        ESP_LOGI(HSERVER_TAG, "Turning on fan");
    } else {
        ESP_LOGE(HSERVER_TAG, "Unknown command!!!");
    }

    const char successResp[] = "Action has been successfuly executed";
    httpd_resp_send(req, successResp, strlen(successResp));
    return ESP_OK;
}
httpd_handle_t http_server_start()
{
    httpd_config_t httpdConfig = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t uri_post =
    {
     .handler = uri_post_handler,
     .method = HTTP_POST,
     .uri = "/fan",
     .user_ctx = NULL
    };
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &httpdConfig) == ESP_OK) {
        ESP_LOGI(HSERVER_TAG, "Successful Server HTTP");
         httpd_register_uri_handler(server, &uri_post);
     }
return server;
}

esp_http_client_handle_t http_client_init()
{
    esp_http_client_config_t httpConf = 
    {
        .url = POST_URL,
        .method = HTTP_METHOD_POST,
        .event_handler = NULL, 
        .timeout_ms = 7500
    };
    esp_http_client_handle_t client = esp_http_client_init(&httpConf);
    return client;
}

void http_client_post(void *pvParameters)
{
    for(;;){
    esp_http_client_handle_t client = http_client_init();

    cJSON *json_data = NULL; 
    char *string_json = NULL;

    json_data = cJSON_CreateObject();

    SensorData_t sensorData;

    esp_err_t result = readData(&sensorData);
    if(result != ESP_OK)
    {
     vTaskDelay(pdMS_TO_TICKS(periodTime));
     continue;
    }

    cJSON_AddNumberToObject(json_data, "temperature", sensorData.temperature);
    cJSON_AddNumberToObject(json_data, "humidity", sensorData.humidity);

    string_json = cJSON_Print(json_data);
    cJSON_Delete(json_data);
    
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, string_json, strlen(string_json));

    esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) 
        {
            ESP_LOGI(HCLIENT_TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
        } else {
            ESP_LOGE(HCLIENT_TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
        }

    free(string_json);
    esp_http_client_cleanup(client);
    vTaskDelay(pdMS_TO_TICKS(periodTime));
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

    actuator_init();

    http_server_start();
    
    xTaskCreate(http_client_post, "http-client", 8192, NULL, 2, NULL);
}