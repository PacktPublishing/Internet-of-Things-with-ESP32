#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "mqtt_client.h"

#include "app_temp.h"
#include "app_wifi.h"

#define TAG "app"

#ifndef MQTT_BROKER_URL
#define MQTT_BROKER_URL "mqtt://<your_broker_url>"
#endif

#define SENSOR_NO "1"
#define ENABLE_TOPIC "home/" SENSOR_NO "/enable"
#define TEMP_TOPIC "home/temperature/" SENSOR_NO
#define HUM_TOPIC "home/humidity/" SENSOR_NO

static esp_mqtt_client_handle_t client = NULL;
static bool enabled = false;

static void publish_reading(int temp, int hum)
{
    char buffer[5];

    if (client != NULL && enabled)
    {
        esp_mqtt_client_publish(client, TEMP_TOPIC, itoa(temp, buffer, 10), 0, 1, 0);
        esp_mqtt_client_publish(client, HUM_TOPIC, itoa(hum, buffer, 10), 0, 1, 0);
    }
}

static void handle_mqtt_events(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "mqtt broker connected");
        esp_mqtt_client_subscribe(client, ENABLE_TOPIC, 0);
        break;
    case MQTT_EVENT_DATA:
        if (!strncmp(event->topic, ENABLE_TOPIC, event->topic_len))
        {
            enabled = event->data[0] - '0';
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "errtype: %d", event->error_handle->error_type);
        break;
    default:
        ESP_LOGI(TAG, "event: %d", event_id);
        break;
    }
}

static void handle_wifi_connect(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_BROKER_URL,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, handle_mqtt_events, NULL);
    esp_mqtt_client_start(client);
    apptemp_init(publish_reading);
}

static void handle_wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main()
{
    esp_event_loop_create_default();

    connect_wifi_params_t cbs = {
        .on_connected = handle_wifi_connect,
        .on_failed = handle_wifi_failed};
    appwifi_connect(cbs);
}