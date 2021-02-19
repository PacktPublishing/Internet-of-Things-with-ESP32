#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "string.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_wifi.h"

#include "wifi_connect.h"

static const char *TAG = "ota_test";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

void exit_ota(const char *mess)
{
    printf("> exiting: %s\n", mess);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}

void start_ota(void *arg)
{
    esp_http_client_config_t config = {
        .url = APP_OTA_URL,
        .cert_pem = (char *)server_cert_pem_start,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    if (esp_https_ota_begin(&ota_config, &https_ota_handle) != ESP_OK)
    {
        exit_ota("esp_https_ota_begin failed");
        return;
    }

    esp_app_desc_t new_app;
    if (esp_https_ota_get_img_desc(https_ota_handle, &new_app) != ESP_OK)
    {
        exit_ota("esp_https_ota_get_img_desc failed");
        return;
    }

    const esp_partition_t *current_partition = esp_ota_get_running_partition();
    esp_app_desc_t existing_app;
    esp_ota_get_partition_description(current_partition, &existing_app);

    ESP_LOGI(TAG, "existing version: '%s'", existing_app.version);
    ESP_LOGI(TAG, "target version: '%s'", new_app.version);

    if (memcmp(new_app.version, existing_app.version, sizeof(new_app.version)) == 0)
    {
        exit_ota("no update");
        return;
    }

    ESP_LOGI(TAG, "updating...");
    while (esp_https_ota_perform(https_ota_handle) == ESP_ERR_HTTPS_OTA_IN_PROGRESS)
        ;

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true)
    {
        exit_ota("download failed");
        return;
    }

    if (esp_https_ota_finish(https_ota_handle) == ESP_OK)
    {
        ESP_LOGI(TAG, "rebooting..");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }

    exit_ota("ota failed");
}

void wifi_conn_cb(void)
{
    xTaskCreate(&start_ota, "ota", 8192, NULL, 5, NULL);
}

void wifi_failed_cb(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main(void)
{
    ESP_LOGI(TAG, "this is 0.0.1");
    connect_wifi_params_t p = {
        .on_connected = wifi_conn_cb,
        .on_failed = wifi_failed_cb};
    connect_wifi(p);

    esp_wifi_set_ps(WIFI_PS_NONE);
}