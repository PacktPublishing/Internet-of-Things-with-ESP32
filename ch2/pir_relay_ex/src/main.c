#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define RELAY_PIN 4
#define GPIO_RELAY_PIN_SEL (1ULL << RELAY_PIN)
#define PIR_PIN 5
#define GPIO_PIR_PIN_SEL (1ULL << PIR_PIN)
#define ESP_INTR_FLAG_DEFAULT 0
#define STATE_CHECK_PERIOD 10000

static void pir_handler(void *arg);

static void init_hw(void)
{
    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_RELAY_PIN_SEL;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_PIR_PIN_SEL;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(PIR_PIN, pir_handler, NULL);
}

static TickType_t next = 0;
const TickType_t period = STATE_CHECK_PERIOD / portTICK_PERIOD_MS;

static void IRAM_ATTR pir_handler(void *arg)
{
    TickType_t now = xTaskGetTickCountFromISR();

    if (now > next)
    {
        gpio_set_level(RELAY_PIN, 1);
    }
    next = now + period;
}

static void open_relay(void *arg)
{
    while (1)
    {
        TickType_t now = xTaskGetTickCount();
        if (now > next)
        {
            gpio_set_level(RELAY_PIN, 0);
            vTaskDelay(period);
        }
        else
        {
            vTaskDelay(next - now);
        }
    }
}

void app_main()
{
    init_hw();
    xTaskCreate(open_relay, "openrl", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}