
#include <stddef.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rotenc.h"

static gpio_num_t clk_pin;
static gpio_num_t dt_pin;

static volatile int rotenc_clk_val = 0;
static volatile int rotenc_pos = 0;
static TaskHandle_t pos_changed_task = NULL;

static void rotenc_isr(void *arg);

void rotenc_init(int clk, int dt)
{
    clk_pin = (gpio_num_t)clk;
    dt_pin = (gpio_num_t)dt;

    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << dt);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << clk);
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    gpio_config(&io_conf);
    rotenc_clk_val = gpio_get_level(clk_pin);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(clk_pin, rotenc_isr, NULL);
}

static void IRAM_ATTR rotenc_isr(void *arg)
{
    int clk_val = gpio_get_level(clk_pin);
    int dt_val = gpio_get_level(dt_pin);

    if (rotenc_clk_val != clk_val)
    {
        rotenc_pos += clk_val == dt_val ? 1 : -1;
        rotenc_clk_val = clk_val;
        if (pos_changed_task != NULL)
        {
            xTaskResumeFromISR(pos_changed_task);
        }
    }
}

int rotenc_getPos(void)
{
    return rotenc_pos;
}

void rotenc_setPosChangedCallback(posChangedCb_t cb)
{
    xTaskCreate(cb, "rpos", configMINIMAL_STACK_SIZE * 5, NULL, 5, &pos_changed_task);
}