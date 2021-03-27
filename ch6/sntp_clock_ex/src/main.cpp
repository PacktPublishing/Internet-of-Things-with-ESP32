#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"
#include "esp_sntp.h"

#include "ssd1306.h"
#include "wifi_connect.h"

#define TAG "sntp_ex"

#define OLED_CLK 22
#define OLED_SDA 21

extern "C" void app_main(void);

static void init_hw(void)
{
    ssd1306_128x64_i2c_initEx(OLED_CLK, OLED_SDA, 0);
}

static void init_sntp(void)
{
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

static void show_time(void *arg)
{
    time_t now = 0;
    struct tm timeinfo;
    memset((void *)&timeinfo, 0, sizeof(timeinfo));
    char buf[64];

    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font8x16);

    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(buf, sizeof(buf), "%a", &timeinfo);
        ssd1306_printFixed(0, 0, buf, STYLE_NORMAL);
        strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
        ssd1306_printFixed(0, 32, buf, STYLE_BOLD);
    }
}

static void sync_time(void *arg)
{
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font8x16);
    ssd1306_printFixed(0, 32, "Running sync...", STYLE_NORMAL);

    int retry = 0;
    const int retry_count = 10;

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if (retry == retry_count)
    {
        ssd1306_clearScreen();
        ssd1306_printFixed(0, 32, "Sync failed.", STYLE_NORMAL);
        vTaskDelete(NULL);
        return;
    }

    xTaskCreate(show_time, "show_time", 8192, NULL, 5, NULL);
    vTaskDelete(NULL);
}

void wifi_conn_cb(void)
{
    init_sntp();
    xTaskCreate(sync_time, "sync", 8192, NULL, 5, NULL);
}

void wifi_failed_cb(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main()
{
    init_hw();
    connect_wifi_params_t p = {
        .on_connected = wifi_conn_cb,
        .on_failed = wifi_failed_cb};
    connect_wifi(p);
}