#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_tls.h"
#include "wifi_connect.h"
#include "private_include/esp_tls_mbedtls.h"

// openssl s_client -showcerts -connect reqbin.com:443 < /dev/null

static const char *TAG = "tls_ex";

static const char REQUEST[] = "GET /echo/get/json HTTP/1.1\r\n"
                              "Host: reqbin.com\r\n"
                              "User-Agent: esp32\r\n"
                              "Accept: */*\r\n"
                              "\r\n";

extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[] asm("_binary_server_cert_pem_end");

static void do_https_get(void *arg)
{
    char buf[512];
    int ret, len;

    esp_tls_cfg_t cfg = {
        .cacert_buf = server_root_cert_pem_start,
        .cacert_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
    };

    struct esp_tls *tls = esp_tls_conn_http_new("https://reqbin.com", &cfg);
    if (tls == NULL)
    {
        ESP_LOGE(TAG, "esp_tls_conn_http_new failed");
        vTaskDelete(NULL);
        return;
    }

    ret = esp_mbedtls_write(tls, REQUEST, strlen(REQUEST));
    if (ret > 0)
    {
        while (1)
        {
            len = sizeof(buf) - 1;
            ret = esp_mbedtls_read(tls, (char *)buf, len);
            if (ret > 0)
            {
                buf[ret] = 0;
                ESP_LOGI(TAG, "%s", buf);
            }
            else
            {
                break;
            }
        }
    }

    esp_tls_conn_delete(tls);
    vTaskDelete(NULL);
}

void wifi_conn_cb(void)
{
    xTaskCreate(&do_https_get, "https", 8192, NULL, 5, NULL);
}

void wifi_failed_cb(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main(void)
{
    connect_wifi_params_t p = {
        .on_connected = wifi_conn_cb,
        .on_failed = wifi_failed_cb};
    connect_wifi(p);
}