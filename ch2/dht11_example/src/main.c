#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <dht.h>
#include "driver/gpio.h"

#define DHT11_PIN 17
#define BUZZER_PIN 18
#define BUZZER_PIN_SEL (1ULL << BUZZER_PIN)

#define HUM_THRESHOLD 800
#define TEMP_THRESHOLD 250

static void init_hw(void)
{
    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = BUZZER_PIN_SEL;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}

static void beep(void *arg)
{
    int cnt = 2 * (int)arg;
    bool state = true;
    for (int i = 0; i < cnt; ++i, state = !state)
    {
        gpio_set_level(BUZZER_PIN, state);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

static int16_t temperature;
static bool temp_alarm = false;
static int16_t humidity;
static bool hum_alarm =false;

static void check_alarm(void)
{
    bool is_alarm = temperature >= TEMP_THRESHOLD;
    bool run_beep = is_alarm && !temp_alarm;
    temp_alarm = is_alarm;
    if (run_beep)
    {
        xTaskCreate(beep, "beep", configMINIMAL_STACK_SIZE, (void *)3, 5, NULL);
        return;
    }

    is_alarm = humidity >= HUM_THRESHOLD;
    run_beep = is_alarm && !hum_alarm;
    hum_alarm = is_alarm;
    if (run_beep)
    {
        xTaskCreate(beep, "beep", configMINIMAL_STACK_SIZE, (void *)2, 5, NULL);
    }
}

int app_main()
{
    init_hw();

    while (1)
    {
        if (dht_read_data(DHT_TYPE_DHT11, (gpio_num_t)DHT11_PIN, &humidity, &temperature) == ESP_OK)
        {
            printf("Humidity: %d%% Temp: %dC\n", humidity / 10, temperature / 10);
            check_alarm();
        }
        else
        {
            printf("Could not read data from sensor\n");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}