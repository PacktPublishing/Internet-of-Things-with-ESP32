
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "app_ble.h"
#include "app_ip.h"
#include "app_web.h"
#include "app_ledattn.h"

#define TAG "gateway"

void wifi_connected(void)
{
    ESP_LOGI(TAG, "wifi connected");
    appweb_start_server();
}

void wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

static void attn_on(void)
{
    appled_set(true);
}
static void attn_off(void)
{
    appled_set(false);
}

void app_main(void)
{
    appled_init();
    appweb_init(appble_set_switch);

    connect_wifi_params_t p = {
        .on_connected = wifi_connected,
        .on_failed = wifi_failed,
    };
    appip_connect_wifi(p);

    appble_attn_cbs_t cbs = {attn_on, attn_off};
    init_ble(cbs);
}