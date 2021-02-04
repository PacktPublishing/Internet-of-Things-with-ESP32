#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <bmp280.h>
#include <string.h>

#define SDA_GPIO 21
#define SCL_GPIO 22

static bmp280_t temp_sensor;

static void init_hw(void)
{
    i2cdev_init();

    memset(&temp_sensor, 0, sizeof(bmp280_t));
    temp_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    bmp280_params_t params;
    bmp280_init_default_params(&params);

    bmp280_init_desc(&temp_sensor, BMP280_I2C_ADDRESS_0, 0, SDA_GPIO, SCL_GPIO);
    bmp280_init(&temp_sensor, &params);
}

void app_main()
{
    init_hw();

    float pressure, temperature, humidity;
    while (1)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        if (bmp280_read_float(&temp_sensor, &temperature, &pressure, &humidity) == ESP_OK)
        {
            printf("%.2f Pa, %.2f C, %.2f %%\n", pressure, temperature, humidity);
        }
    }
}
