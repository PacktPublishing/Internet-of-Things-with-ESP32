#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ds18x20.h>

#define SENSOR_PIN 21

#define MAX_SENSORS 8
static ds18x20_addr_t addrs[MAX_SENSORS];
static int sensor_count = 0;
static float temps[MAX_SENSORS];

static void init_hw(void)
{
    while (sensor_count == 0)
    {
        sensor_count = ds18x20_scan_devices((gpio_num_t)SENSOR_PIN, addrs, MAX_SENSORS);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (sensor_count > MAX_SENSORS)
    {
        sensor_count = MAX_SENSORS;
    }
}

void app_main()
{
    init_hw();

    while (1)
    {
        ds18x20_measure_and_read_multi((gpio_num_t)SENSOR_PIN, addrs, sensor_count, temps);
        for (int i = 0; i < sensor_count; i++)
        {
            printf("sensor-id: %08x temp: %fC\n", (uint32_t)addrs[i], temps[i]);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
