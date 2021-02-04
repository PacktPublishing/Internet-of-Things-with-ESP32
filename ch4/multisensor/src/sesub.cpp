#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <tsl2561.h>
#include <bmp280.h>
#include <esp_err.h>
#include <string.h>

#include "sesub.h"

static tsl2561_t light_sensor;
static bmp280_t temp_sensor;
static sesub_config_t config;

static void read_ambient(void *arg);

void sesub_init(sesub_config_t c)
{
    config = c;

    i2cdev_init();

    memset(&light_sensor, 0, sizeof(tsl2561_t));
    light_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    tsl2561_init_desc(&light_sensor, TSL2561_I2C_ADDR_FLOAT, 0, (gpio_num_t)c.sensor_sda, (gpio_num_t)c.sensor_scl);
    tsl2561_init(&light_sensor);

    memset(&temp_sensor, 0, sizeof(bmp280_t));
    temp_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    bmp280_params_t params;
    bmp280_init_default_params(&params);

    bmp280_init_desc(&temp_sensor, BMP280_I2C_ADDRESS_0, 0, (gpio_num_t)c.sensor_sda, (gpio_num_t)c.sensor_scl);
    bmp280_init(&temp_sensor, &params);
}

void sesub_start(void) {
    xTaskCreate(read_ambient, "read", 5 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}

static void read_ambient(void *arg)
{
    float pressure, temperature, humidity;
    uint32_t lux;

    while (1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(bmp280_read_float(&temp_sensor, &temperature, &pressure, &humidity));
        ESP_ERROR_CHECK(tsl2561_read_lux(&light_sensor, &lux));
        if (temperature > config.temp_high || temperature < config.temp_low)
        {
            if (config.temp_alarm)
            {
                config.temp_alarm();
            }
        }
        if (config.new_sensor_reading)
        {
            sensor_reading_t reading = {(int)temperature, (int)humidity, (int)(pressure / 1000), (int)lux};
            config.new_sensor_reading(reading);
        }
    }
}