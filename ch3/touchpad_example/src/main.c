
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/touch_pad.h"
#include <string.h>

static portMUX_TYPE mut = portMUX_INITIALIZER_UNLOCKED;

typedef struct
{
    int pin_num;
    TickType_t when;
} touch_info_t;

#define TI_LIST_SIZE 10
static volatile touch_info_t ti_list[TI_LIST_SIZE];
static volatile size_t ti_cnt = 0;

static const TickType_t check_period = 500 / portTICK_PERIOD_MS;

static void IRAM_ATTR tp_handler(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status();
    touch_pad_clear_status();

    touch_info_t touch = {
        .pin_num = (pad_intr >> TOUCH_PAD_NUM8) & 0x01 ? TOUCH_PAD_NUM8 : TOUCH_PAD_NUM9,
        .when = xTaskGetTickCountFromISR()};

    portENTER_CRITICAL_SAFE(&mut);
    if (ti_cnt < TI_LIST_SIZE)
    {
        bool skip = (ti_cnt > 0) &&
                    ((touch.when - ti_list[ti_cnt - 1].when) < check_period) &&
                    (touch.pin_num == ti_list[ti_cnt - 1].pin_num);
        if (!skip)
        {
            ti_list[ti_cnt++] = touch;
        }
    }
    portEXIT_CRITICAL_SAFE(&mut);
}

static void monitor(void *arg)
{
    touch_info_t ti_list_local[TI_LIST_SIZE];
    size_t ti_cnt_local;

    while (1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ti_cnt_local = 0;

        portENTER_CRITICAL_SAFE(&mut);
        if (ti_cnt > 0)
        {
            memcpy((void *)ti_list_local, (const void *)ti_list, ti_cnt * sizeof(touch_info_t));
            ti_cnt_local = ti_cnt;
            ti_cnt = 0;
        }
        portEXIT_CRITICAL_SAFE(&mut);

        if (ti_cnt_local > 0)
        {
            int t8_cnt = 0;
            for (int i = 0; i < ti_cnt_local; ++i)
            {
                if (ti_list_local[i].pin_num == TOUCH_PAD_NUM8)
                {
                    ++t8_cnt;
                }
            }
            printf("First touch tick: %u\n", ti_list_local[0].when);
            printf("Last touch tick: %u\n", ti_list_local[ti_cnt_local - 1].when);
            printf("Touch8 count: %d\n", t8_cnt);
            printf("Touch9 count: %d\n", ti_cnt_local - t8_cnt);
        }
        else
        {
            printf("No touch detected\n");
        }
    }
}

static void init_hw(void)
{
    touch_pad_init();
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);

    touch_pad_config(TOUCH_PAD_NUM8, 0);
    touch_pad_config(TOUCH_PAD_NUM9, 0);
    touch_pad_filter_start(10);

    uint16_t val;
    touch_pad_read_filtered(TOUCH_PAD_NUM8, &val);
    touch_pad_set_thresh(TOUCH_PAD_NUM8, val * 0.2);
    touch_pad_read_filtered(TOUCH_PAD_NUM9, &val);
    touch_pad_set_thresh(TOUCH_PAD_NUM9, val * 0.2);

    touch_pad_isr_register(tp_handler, NULL);
}

void app_main(void)
{
    init_hw();

    TaskHandle_t taskh;
    if (xTaskCreatePinnedToCore(monitor,
                                "monitor",
                                1024,
                                NULL,
                                2,
                                &taskh,
                                APP_CPU_NUM) == pdPASS)
    {
        printf("info: monitor started\n");
    }
    else
    {
        printf("err: monitor task couldn't start\n");
    }
    char buffer[128];
    vTaskList(buffer);
    printf("%s\n", buffer);

    touch_pad_intr_enable();
}
