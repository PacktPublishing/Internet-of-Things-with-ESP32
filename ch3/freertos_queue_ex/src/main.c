#include <inttypes.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define TILT_SWITCH_PIN 16
#define SHOCK_SWITCH_PIN 5
#define TAP_SWITCH_PIN 17

#define MS_100 100000

typedef struct
{
    gpio_num_t pin;
    int64_t time;
} queue_data_t;

static QueueHandle_t sensor_event_queue = NULL;

static bool filter_out(queue_data_t *);

static void IRAM_ATTR producer(void *arg)
{
    queue_data_t data = {
        .pin = (uint32_t)arg,
        .time = esp_timer_get_time()};
    xQueueSendToBackFromISR(sensor_event_queue, &data, NULL);
}

static void consumer(void *arg)
{
    queue_data_t data;
    while (1)
    {
        if (xQueueReceive(sensor_event_queue, &data, portMAX_DELAY))
        {
            if (filter_out(&data))
            {
                continue;
            }
            switch (data.pin)
            {
            case SHOCK_SWITCH_PIN:
                printf("> shock sensor");
                break;
            case TILT_SWITCH_PIN:
                printf("> tilt sensor");
                break;
            case TAP_SWITCH_PIN:
                printf("> tap sensor");
                break;
            default:
                break;
            }
            printf(" at %" PRId64 "(us)\n", data.time);
        }
    }
    vTaskDelete(NULL);
}

static bool filter_out(queue_data_t *d)
{
    static int64_t tilt_time = 0;
    static int64_t tap_time = 0;
    static int64_t shock_time = 0;

    switch (d->pin)
    {
    case TILT_SWITCH_PIN:
        if (d->time - tilt_time < MS_100)
        {
            return true;
        }
        tilt_time = d->time;
        break;
    case TAP_SWITCH_PIN:
        if (d->time - tap_time < MS_100)
        {
            return true;
        }
        tap_time = d->time;
        break;
    case SHOCK_SWITCH_PIN:
        if (d->time - shock_time < MS_100)
        {
            return true;
        }
        shock_time = d->time;
        break;
    default:
        break;
    }

    return false;
}

static void init_hw(void)
{
    uint64_t pin_select = 0;
    pin_select |= (1ULL << SHOCK_SWITCH_PIN);
    pin_select |= (1ULL << TILT_SWITCH_PIN);
    pin_select |= (1ULL << TAP_SWITCH_PIN);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    io_conf.pin_bit_mask = pin_select;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(SHOCK_SWITCH_PIN, producer, (void *)SHOCK_SWITCH_PIN);
    gpio_isr_handler_add(TILT_SWITCH_PIN, producer, (void *)TILT_SWITCH_PIN);
    gpio_isr_handler_add(TAP_SWITCH_PIN, producer, (void *)TAP_SWITCH_PIN);
}

void app_main(void)
{
    init_hw();
    sensor_event_queue = xQueueCreate(20, sizeof(queue_data_t));
    xTaskCreate(consumer, "consumer", 2048, NULL, 10, NULL);
}