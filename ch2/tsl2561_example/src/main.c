#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <tsl2561.h>
#include "driver/ledc.h"

#define SDA_GPIO 21
#define SCL_GPIO 22
#define ADDR TSL2561_I2C_ADDR_FLOAT
static tsl2561_t light_sensor;

#define LEDC_GPIO 18
static ledc_channel_config_t ledc_channel;

static void init_hw(void)
{
    i2cdev_init();

    memset(&light_sensor, 0, sizeof(tsl2561_t));
    light_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    tsl2561_init_desc(&light_sensor, ADDR, 0, SDA_GPIO, SCL_GPIO);
    tsl2561_init(&light_sensor);

    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 1000,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.duty = 0;
    ledc_channel.gpio_num = LEDC_GPIO;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.hpoint = 0;
    ledc_channel.timer_sel = LEDC_TIMER_0;
    ledc_channel_config(&ledc_channel);
}

static void set_led(uint32_t lux)
{
    uint32_t duty = 1023;
    if (lux > 50)
    {
        duty = 0;
    }
    else if (lux > 20)
    {
        duty /= 2;
    }

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, duty);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}

void app_main()
{
    init_hw();

    uint32_t lux;
    while (1)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (tsl2561_read_lux(&light_sensor, &lux) == ESP_OK)
        {
            printf("Lux: %u\n", lux);
            set_led(lux);
        }
    }
}
