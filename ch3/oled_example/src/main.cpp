
/*
original library: https://github.com/lexus2k/ssd1306
*/

#include "dht.h"
#include "ssd1306.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#define DHT11_PIN 17
#define OLED_CLK 22
#define OLED_SDA 21

extern "C" void app_main(void);

static void init_hw(void)
{
    ssd1306_128x64_i2c_initEx(OLED_CLK, OLED_SDA, 0);
}

static void draw_screen(void)
{
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font8x16);
    ssd1306_printFixed(0, 0, "Temp", STYLE_NORMAL);
    ssd1306_printFixed(0, 32, "Hum", STYLE_NORMAL);
}

static void display_reading(int temp, int hum)
{
    char buff[10];
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    sprintf(buff, "%d", temp);
    ssd1306_printFixedN(48, 0, buff, STYLE_BOLD, 2);

    sprintf(buff, "%d", hum);
    ssd1306_printFixedN(48, 32, buff, STYLE_BOLD, 2);
}

static void read_dht11(void* arg)
{
    int16_t humidity = 0, temperature = 0;
    while(1)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        dht_read_data(DHT_TYPE_DHT11, (gpio_num_t)DHT11_PIN, &humidity, &temperature);
        display_reading(temperature / 10, humidity / 10);
    }
}

void app_main()
{
    init_hw();
    draw_screen();

    xTaskCreate(read_dht11, "dht11", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);
}
