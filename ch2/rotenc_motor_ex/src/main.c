#include <stdio.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rotenc.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#define ROTENC_CLK_PIN 19
#define ROTENC_DT_PIN 21

#define MOTOR_DIR_PIN 22
#define MOTOR_STEP_PIN 23

static void init_hw(void)
{
    rotenc_init(ROTENC_CLK_PIN, ROTENC_DT_PIN);

    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    io_conf.pin_bit_mask = 0;
    io_conf.pin_bit_mask |= (1ULL << MOTOR_DIR_PIN);
    gpio_config(&io_conf);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_STEP_PIN);
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 250;
    pwm_config.cmpr_a = 0;
    pwm_config.cmpr_b = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

static int rotenc_pos = 0;
static int motor_pos = 0;

static void print_rotenc_pos(void *arg)
{
    while (1)
    {
        rotenc_pos = rotenc_getPos();
        printf("pos: %d\n", rotenc_pos);
        vTaskSuspend(NULL);
    }
}

void app_main()
{
    init_hw();
    rotenc_setPosChangedCallback(print_rotenc_pos);

    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, 4000);
    int steps;
    int step_delay = 100;

    while (1)
    {
        if (motor_pos == rotenc_pos)
        {
            vTaskDelay(100);
            continue;
        }
        steps = rotenc_pos - motor_pos;
        motor_pos = rotenc_pos;

        gpio_set_level((gpio_num_t)MOTOR_DIR_PIN, steps > 0);
        vTaskDelay(10);

        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, 50);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
        vTaskDelay(step_delay * abs(steps) / portTICK_PERIOD_MS);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A);
    }
}
