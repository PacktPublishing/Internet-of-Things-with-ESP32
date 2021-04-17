#include "app_temp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht.h"
#include "esp_log.h"

#include <stdint.h>

#define TAG "app.temp"

static temp_ready_f temp_cb;

static void read_temp(void *arg)
{
    int16_t humidity, temperature;

    while (1)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        if (dht_read_data(DHT_TYPE_DHT11, (gpio_num_t)DHT11_PIN, &humidity, &temperature) == ESP_OK)
        {
            humidity /= 10;
            temperature /= 10;
            ESP_LOGD(TAG, "Humidity: %d%% Temp: %dC", humidity, temperature);
            if (temp_cb)
            {
                temp_cb(temperature, humidity);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Could not read data from sensor");
        }
    }
}

void apptemp_init(temp_ready_f cb)
{
    temp_cb = cb;
    xTaskCreate(read_temp, TAG, 3 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}