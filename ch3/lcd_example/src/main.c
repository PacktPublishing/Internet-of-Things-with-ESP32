
// Original library: https://github.com/maxsydney/ESP32-HD44780

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <stdio.h>
#include <driver/i2c.h>
#include <HD44780.h>
#include <dht.h>

#define LCD_ADDR 0x27
#define SDA_PIN 21
#define SCL_PIN 22
#define LCD_COLS 16
#define LCD_ROWS 2

#define DHT11_PIN 17

static void init_hw(void)
{
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
}

void show_dht11(void *param)
{
    char buff[17];
    int16_t temperature, humidity;
    uint8_t read_cnt = 0;

    while (true)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        LCD_clearScreen();

        if (dht_read_data(DHT_TYPE_DHT11, (gpio_num_t)DHT11_PIN, &humidity, &temperature) == ESP_OK)
        {
            temperature /= 10;
            humidity /= 10;

            memset(buff, 0, sizeof(buff));
            sprintf(buff, "Temp: %d", temperature);
            LCD_home();
            LCD_writeStr(buff);

            memset(buff, 0, sizeof(buff));
            sprintf(buff, "Hum: %d", humidity);
            LCD_setCursor(0, 1);
            LCD_writeStr(buff);
        }
        else
        {
            LCD_home();
            memset(buff, 0, sizeof(buff));
            sprintf(buff, "Failed (%d)", ++read_cnt);
            LCD_writeStr(buff);
        }
    }
}

void app_main(void)
{
    init_hw();
    xTaskCreate(show_dht11, "dht11", configMINIMAL_STACK_SIZE * 4, NULL, 5, NULL);
}
