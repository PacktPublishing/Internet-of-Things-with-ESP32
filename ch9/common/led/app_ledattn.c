#include "app_ledattn.h"
#include "driver/gpio.h"

void appled_init(void)
{
    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << ATTN_LED_PIN);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 1;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

void appled_set(bool state)
{
    gpio_set_level((gpio_num_t)ATTN_LED_PIN, (uint32_t)state);
}
