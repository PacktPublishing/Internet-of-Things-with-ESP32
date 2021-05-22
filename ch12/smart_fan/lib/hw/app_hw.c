#include "app_hw.h"
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define BTN_CNT 4
#define TAG "app.btn"

static appbtn_fan_changed_f fan_cb = NULL;
static btn_map_t btn_map[BTN_CNT] = {
    {APP_BTN0, GPIO_NUM_NC, 0},
    {APP_BTN1, APP_RELAY1, 33},
    {APP_BTN2, APP_RELAY2, 66},
    {APP_BTN3, APP_RELAY3, 100}};

static uint8_t active_btn_idx = 0;

static QueueHandle_t idx_queue = NULL;
static TickType_t next = 0;

static void IRAM_ATTR button_handler(void *arg)
{
    TickType_t now = xTaskGetTickCountFromISR();
    if (next > now)
    {
        return;
    }
    next = now + 100 / portTICK_PERIOD_MS;
    int idx = (int)(arg);
    xQueueSendToBackFromISR(idx_queue, &idx, NULL);
}

static void set_relay_task(void *arg)
{
    uint8_t idx;
    gpio_set_level(APP_OE, 1);

    while (1)
    {
        if (xQueueReceive(idx_queue, &idx, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "received idx: %d", idx);

            if (idx == active_btn_idx)
            {
                continue;
            }
            for (int i = 1; i < BTN_CNT; ++i)
            {
                gpio_set_level(btn_map[i].relay_pin, 1);
            }

            if (idx != 0)
            {
                ESP_LOGI(TAG, "setting idx: %d", idx);
                vTaskDelay(250 / portTICK_RATE_MS);
                gpio_set_level(btn_map[idx].relay_pin, 0);
            }

            active_btn_idx = idx;
            if (fan_cb != NULL)
            {
                fan_cb(btn_map[idx].val);
            }
        }
    }
    vTaskDelete(NULL);
}

void apphw_init(appbtn_fan_changed_f f)
{
    fan_cb = f;

    gpio_config_t io_conf;
    uint64_t pin_sel = 0;

    pin_sel = (1ULL << APP_OE);
    pin_sel |= (1ULL << APP_RELAY1);
    pin_sel |= (1ULL << APP_RELAY2);
    pin_sel |= (1ULL << APP_RELAY3);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = pin_sel;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    pin_sel = (1ULL << APP_BTN0);
    pin_sel |= (1ULL << APP_BTN1);
    pin_sel |= (1ULL << APP_BTN2);
    pin_sel |= (1ULL << APP_BTN3);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = pin_sel;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    for (int i = 0; i < BTN_CNT; ++i)
    {
        gpio_isr_handler_add(btn_map[i].btn_pin, button_handler, (void *)i);
    }

    gpio_set_level(APP_OE, 0);
    for (int i = 1; i < BTN_CNT; ++i)
    {
        gpio_set_level(btn_map[i].relay_pin, 1);
    }

    idx_queue = xQueueCreate(3, sizeof(int));
    xTaskCreatePinnedToCore(set_relay_task, "consumer", 2048, NULL, 10, NULL, 0);
}

uint8_t apphw_get_state(void)
{
    return btn_map[active_btn_idx].val;
}

void apphw_set_state(uint8_t val)
{
    if (val == 0)
    {
        xQueueSendToBack(idx_queue, &val, 0);
    }
    else
    {
        for (int i = 1; i < BTN_CNT; ++i)
        {
            if (val <= btn_map[i].val)
            {
                xQueueSendToBack(idx_queue, &i, 0);
                break;
            }
        }
    }
}
