#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <string.h>
#include <esp_sleep.h>
#include "pmsub.h"

static pmsub_config_t config;
static volatile TickType_t next_sleep = 0;
static const TickType_t period = 20000 / portTICK_PERIOD_MS;

static void IRAM_ATTR pir_handler(void *);
static void sleep_check(void *arg);

void pmsub_init(pmsub_config_t c)
{
    config = c;

    gpio_config_t io_conf;
    memset((void *)&io_conf, 0, sizeof(io_conf));
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << c.pir_pin);
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add((gpio_num_t)c.pir_pin, pir_handler, NULL);
    gpio_wakeup_enable((gpio_num_t)c.pir_pin, GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    pmsub_update(false);
}

void pmsub_update(bool from_isr)
{
    if (from_isr)
    {
        next_sleep = period + xTaskGetTickCountFromISR();
    }
    else
    {
        next_sleep = period + xTaskGetTickCount();
    }
}

void pmsub_start(void)
{
    xTaskCreate(sleep_check, "sleep", 3 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}

static void sleep_check(void *arg)
{
    while (1)
    {
        TickType_t now = xTaskGetTickCount();
        if (now > next_sleep)
        {
            if (config.before_sleep)
            {
                config.before_sleep();
            }
            esp_light_sleep_start();

            if (config.after_wakeup)
            {
                config.after_wakeup();
            }
            pmsub_update(false);
            now = xTaskGetTickCount();
        }
        vTaskDelay(next_sleep - now);
    }
}

static void IRAM_ATTR pir_handler(void *arg)
{
    pmsub_update(true);
}
