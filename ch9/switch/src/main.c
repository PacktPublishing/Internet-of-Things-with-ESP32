
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "app_ble.h"
#include "app_sw.h"
#include "app_ledattn.h"

#define TAG "switch"

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
    init_hw();
    appled_init();

    app_ble_cb_t cbs = {
        .sw_set = switch_set,
        .sw_get = switch_get,
        .attn_on = attn_on,
        .attn_off = attn_off,
    };
    init_ble(cbs);
}