
#include "wifi_connect.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "string.h"
#include <stdio.h>

#define MAX_RETRY 10
static int retry_cnt = 0;
static connect_wifi_params_t m_params;

static void handle_wifi_connection(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry_cnt++ < MAX_RETRY)
        {
            esp_wifi_connect();
        }
        else
        {
            if (m_params.on_failed)
            {
                m_params.on_failed();
            }
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        if (m_params.on_connected)
        {
            m_params.on_connected();
        }
    }
}

#ifndef WIFI_SSID
#define WIFI_SSID "ssid"
#define WIFI_PASS "passwd"
#endif

void connect_wifi(connect_wifi_params_t p)
{
    m_params = p;

    if (nvs_flash_init() != ESP_OK)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handle_wifi_connection, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &handle_wifi_connection, NULL));

    wifi_config_t wifi_config;

    memset((void *)&wifi_config, 0, sizeof(wifi_config_t));
    printf("%s/%s\n", WIFI_SSID, WIFI_PASS);
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}