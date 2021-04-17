// openssl s_client -showcerts -connect maker.ifttt.com:443 last cert

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "private_include/esp_tls_mbedtls.h"

#include "app_temp.h"
#include "app_wifi.h"

#define TAG "app"
#define IFTTT_MAKER_URL "https://maker.ifttt.com"

#define QUEUE_SIZE 10
static QueueHandle_t temp_queue;

static const char REQUEST[] = "POST /trigger/temperature_received/with/key/" IFTTT_KEY " HTTP/1.1\r\n"
                              "Host: maker.ifttt.com\r\n"
                              "Content-Type: application/json\r\n"
                              "Content-Length: %d\r\n"
                              "\r\n"
                              "%s";
static const char JSON_DATA[] = "{\"value1\":\"%d\"}";

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[] asm("_binary_server_cert_pem_end");

static void do_post(void *arg)
{
    esp_tls_cfg_t cfg = {
        .cacert_buf = server_root_cert_pem_start,
        .cacert_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
    };

    int16_t temp;
    char json_data[32];
    char request[256];
    char reply[512];

    while (1)
    {
        if (xQueueReceive(temp_queue, &(temp), (TickType_t)10) == pdFALSE)
        {
            ESP_LOGI(TAG, "nothing in the queue");
            vTaskDelay(1000);
            continue;
        }

        struct esp_tls *tls = esp_tls_conn_http_new(IFTTT_MAKER_URL, &cfg);
        if (tls == NULL)
        {
            ESP_LOGE(TAG, "tls connection failed");
            continue;
        }

        memset(json_data, 0, sizeof(json_data));
        sprintf(json_data, JSON_DATA, temp);

        memset(request, 0, sizeof(request));
        sprintf(request, REQUEST, strlen(json_data), json_data);

        int ret = esp_mbedtls_write(tls, request, strlen(request));
        if (ret > 0)
        {
            while (1)
            {
                ret = esp_mbedtls_read(tls, (char *)reply, sizeof(reply) - 1);
                if (ret > 0)
                {
                    reply[ret] = 0;
                    ESP_LOGI(TAG, "%s", reply);
                }
                else
                {
                    break;
                }
            }
        }
        esp_tls_conn_delete(tls);
    }

    vTaskDelete(NULL);
}

static void publish_reading(int temp, int hum)
{
    if (xQueueSendToBack(temp_queue, (void *)&temp, (TickType_t)0) != pdPASS)
    {
        ESP_LOGW(TAG, "queue is full");
        xQueueReset(temp_queue);
    }
}

static void handle_wifi_connect(void)
{
    xTaskCreate(do_post, "post_task", 15 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    apptemp_init(publish_reading);
}

static void handle_wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main()
{
    temp_queue = xQueueCreate(QUEUE_SIZE, sizeof(int16_t));
    connect_wifi_params_t cbs = {
        .on_connected = handle_wifi_connect,
        .on_failed = handle_wifi_failed};
    appwifi_connect(cbs);
}
