
#include "esp_sleep.h"
#include "driver/touch_pad.h"
#include "esp_timer.h"
#include <string.h>
#include <stdio.h>

#define SEC_MULTIPLIER 1000000l

static void init_hw(void)
{
    touch_pad_init();
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    touch_pad_config(TOUCH_PAD_NUM8, 0);
    touch_pad_filter_start(10);

    uint16_t val;
    touch_pad_read_filtered(TOUCH_PAD_NUM8, &val);
    touch_pad_set_thresh(TOUCH_PAD_NUM8, val * 0.2);
}

void app_main(void)
{
    init_hw();
    int cnt = 0;

    while (1)
    {
        esp_sleep_enable_timer_wakeup(5 * SEC_MULTIPLIER);
        esp_sleep_enable_touchpad_wakeup();

        esp_light_sleep_start();

        printf("cnt: %d\n", ++cnt);
        printf("active at (timer value): %lli\n", esp_timer_get_time() / SEC_MULTIPLIER);
        printf("wakeup source: ");

        switch (esp_sleep_get_wakeup_cause())
        {
        case ESP_SLEEP_WAKEUP_TIMER:
            printf("timer\n");
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
        {
            touch_pad_t tp;
            touch_pad_get_wakeup_status(&tp);
            printf("touchpad (%d)\n", tp);
            break;
        }
        default:
            printf("err: no other configured\n");
            break;
        }
    }
}
