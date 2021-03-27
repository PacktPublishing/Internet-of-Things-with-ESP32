#include "app_sw.h"

#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static bool m_status;

void init_hw(void)
{
    m_status = false;

    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << RELAY_PIN);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void switch_set(bool status)
{
    gpio_set_level(RELAY_PIN, (uint32_t)status);
    m_status = status;
}

bool switch_get(void)
{
    return m_status;
}
