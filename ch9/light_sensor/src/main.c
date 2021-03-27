
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

#include "app_sensor.h"
#include "app_ble.h"
#include "app_ledattn.h"

#define TAG "sensor"

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
    init_hw(appble_update_state);
    appled_init();

    appble_attn_cbs_t cbs = {attn_on, attn_off};
    init_ble(cbs);
}
