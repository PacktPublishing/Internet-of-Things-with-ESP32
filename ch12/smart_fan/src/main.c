#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app_wifi.h"
#include "app_hw.h"
#include "app_aws.h"

#define TAG "app"

static void handle_wifi_connect(void)
{
    xTaskCreatePinnedToCore(appaws_connect, "appaws_connect", 15 * configMINIMAL_STACK_SIZE, NULL, 5, NULL, 0);
}

static void handle_wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main()
{
    apphw_init(appaws_publish);
    appaws_init(apphw_set_state);

    connect_wifi_params_t cbs = {
        .on_connected = handle_wifi_connect,
        .on_failed = handle_wifi_failed};
    appwifi_connect(cbs);
}