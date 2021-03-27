
#include "app_sensor.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


bool light_level_low;
static light_changed_f light_changed_cb;

static tsl2561_t light_sensor;

#define TAG "app.sensor"

static void read_light(void *arg)
{
    uint32_t lux;
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (tsl2561_read_lux(&light_sensor, &lux) == ESP_OK)
        {
            ESP_LOGI(TAG, "Lux: %u\n", lux);
            if (lux < LIGHT_THRESHOLD && !light_level_low)
            {
                light_level_low = true;
                if (light_changed_cb != NULL)
                {
                    light_changed_cb(light_level_low);
                }
            }
            else if (lux > LIGHT_THRESHOLD && light_level_low)
            {
                light_level_low = false;
                if (light_changed_cb != NULL)
                {
                    light_changed_cb(light_level_low);
                }
            }
        }
    }
}

void init_hw(light_changed_f f)
{
    light_changed_cb = f;
    light_level_low = false;

    i2cdev_init();

    memset(&light_sensor, 0, sizeof(tsl2561_t));
    light_sensor.i2c_dev.timeout_ticks = 0xffff / portTICK_PERIOD_MS;

    tsl2561_init_desc(&light_sensor, LIGHT_ADDR, 0, LIGHT_SDA, LIGHT_SCL);
    tsl2561_init(&light_sensor);

    xTaskCreate(read_light, "light", 3 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}

bool is_light_low(void)
{
    return light_level_low;
}