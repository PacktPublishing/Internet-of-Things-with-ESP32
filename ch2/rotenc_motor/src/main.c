#include <stdio.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stepper.h"
#include "rotenc.h"

#define ROTENC_CLK_PIN 4
#define ROTENC_DT_PIN 5

#define MOTOR_PIN1 26
#define MOTOR_PIN2 25
#define MOTOR_PIN3 33
#define MOTOR_PIN4 32
#define MOTOR_STEPS_PER_REV 4076

// see this https://lastminuteengineers.com/28byj48-stepper-motor-arduino-tutorial/

static void init_hw(void)
{
    rotenc_init(ROTENC_CLK_PIN, ROTENC_DT_PIN);
    stepper_init(MOTOR_STEPS_PER_REV, MOTOR_PIN1, MOTOR_PIN2, MOTOR_PIN3, MOTOR_PIN4);
}

static TaskHandle_t print_task = NULL;
static volatile int curr_rotenc_pos = 0;
static int last_processed_pos = 0;

static void print_rotenc_pos(void *arg)
{
    while (1)
    {
        curr_rotenc_pos = rotenc_getPos();
        printf("pos: %d\n", curr_rotenc_pos);
        vTaskSuspend(NULL);
    }
}

void app_main()
{
    init_hw();

    xTaskCreate(print_rotenc_pos, "rpos", configMINIMAL_STACK_SIZE * 5, NULL, 5, &print_task);
    rotenc_setPosChangedTask(print_task);

    // stepper_setSpeed(10);

    // int steps;
    // while (1)
    // {
    //     steps = curr_rotenc_pos - last_processed_pos;
    //     if (steps != 0)
    //     {
    //         printf("steps: %d\n", steps);
    //         stepper_step(steps * 10);
    //         last_processed_pos += steps;
    //     }
    //     vTaskDelay(200 / portTICK_PERIOD_MS);
    // }

    stepper_setSpeed(1);
    while (1)
    {
        stepper_step(1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        stepper_step(-1000);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
