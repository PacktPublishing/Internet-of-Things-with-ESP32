
#include "dht.h"
#include "ssd1306.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>

#define DHT11_PIN 17
#define TFT_CS_PIN 5
#define TFT_DC_PIN 21
#define TFT_RST_PIN 22

#define TFT_ROTATE_CW90 (1 & 0x03)

extern "C" void app_main(void);

static void init_hw(void)
{
    st7735_128x160_spi_init(TFT_RST_PIN, TFT_CS_PIN, TFT_DC_PIN);
    ssd1306_setMode(LCD_MODE_NORMAL);
    st7735_setRotation(TFT_ROTATE_CW90);
}

static void draw_screen(void)
{
    ssd1306_clearScreen8();
    ssd1306_setFixedFont(ssd1306xled_font8x16);
    ssd1306_setColor(RGB_COLOR8(255, 255, 255));
    ssd1306_printFixed8(5, 5, "Temperature", STYLE_NORMAL);
    ssd1306_printFixed8(10, 21, "(C)", STYLE_NORMAL);
    ssd1306_printFixed8(5, 64, "Humidity", STYLE_NORMAL);
    ssd1306_printFixed8(10, 80, "(%)", STYLE_NORMAL);
}

static void display_reading(int temp, int hum)
{
    char buff[10];
    ssd1306_setFixedFont(comic_sans_font24x32_123);
    ssd1306_setColor(RGB_COLOR8(255, 0, 0));

    sprintf(buff, "%d", temp);
    ssd1306_printFixed8(80, 21, buff, STYLE_BOLD);

    sprintf(buff, "%d", hum);
    ssd1306_printFixed8(80, 80, buff, STYLE_BOLD);
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
