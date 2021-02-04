
#include "stepper.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static gpio_num_t motor_pins[4];
static int pos;
static int direction;
static TickType_t step_delay;
static int number_of_steps;

static const int steps[4][4] = {{1, 0, 1, 0}, {0, 1, 1, 0}, {0, 1, 0, 1}, {1, 0, 0, 1}};

void stepper_init(int ns, int p1, int p2, int p3, int p4)
{
    pos = 0;
    direction = 0;
    number_of_steps = ns;
    stepper_setSpeed(5);

    motor_pins[0] = (gpio_num_t)p1;
    motor_pins[1] = (gpio_num_t)p2;
    motor_pins[2] = (gpio_num_t)p3;
    motor_pins[3] = (gpio_num_t)p4;

    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    io_conf.pin_bit_mask = 0;
    for (int i = 0; i < 4; ++i)
    {
        io_conf.pin_bit_mask |= (1ULL << motor_pins[i]);
    }
    gpio_config(&io_conf);
}

void stepper_setSpeed(long rev_per_min)
{
    step_delay = 60L * 1000L / number_of_steps / rev_per_min / portTICK_PERIOD_MS;
    if (step_delay == 0)
    {
        step_delay = 1;
    }
}

int stepper_getPos(void)
{
    return pos;
}

static void stepMotor(int thisStep)
{
    for (int i = 0; i < 4; ++i)
    {
        gpio_set_level(motor_pins[i], steps[thisStep][i]);
    }
}

void stepper_step(int steps_to_move)
{
    direction = steps_to_move > 0 ? 1 : -1;
    printf("direction: %d, delay: %u, pos: %d\n", direction, step_delay, pos);

    for (int i = 0; i < abs(steps_to_move); ++i)
    {
        pos += direction;
        pos %= number_of_steps;
        stepMotor(pos % 4);
        vTaskDelay(step_delay);
    }
}
