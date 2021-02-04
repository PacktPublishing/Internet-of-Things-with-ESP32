

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tftspi.h"
#include "tft.h"
#include "dht.h"

/*

Original TFT library: https://github.com/loboris/ESP32_TFT_library

ESP32                       AZ-delivery TFT
#define PIN_NUM_MISO 19		NC
#define PIN_NUM_MOSI 23		SDA
#define PIN_NUM_CLK  18		SCK
#define PIN_NUM_CS   5		CS
#define PIN_NUM_DC   26		A0
#define PIN_NUM_TCS  25		NC
                            LED 3v3
                            RESET + pullup + 3v3
                            GND
                            VCC 3v3

*/

#define DHT11_PIN 17

void init_hw(void)
{
    tft_disp_type = DEFAULT_DISP_TYPE;
    _width = 128;  // smaller dimension
    _height = 160; // larger dimension
    max_rdclock = 8000000;
    TFT_PinsInit();

    spi_lobo_device_handle_t spi;
    spi_lobo_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO, // set SPI MISO pin
        .mosi_io_num = PIN_NUM_MOSI, // set SPI MOSI pin
        .sclk_io_num = PIN_NUM_CLK,  // set SPI CLK pin
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 6 * 1024,
    };
    spi_lobo_device_interface_config_t devcfg = {
        .clock_speed_hz = 8000000,         // Initial clock out at 8 MHz
        .mode = 0,                         // SPI mode 0
        .spics_io_num = -1,                // we will use external CS pin
        .spics_ext_io_num = PIN_NUM_CS,    // external CS pin
        .flags = LB_SPI_DEVICE_HALFDUPLEX, // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

    spi_lobo_bus_add_device(TFT_HSPI_HOST, &buscfg, &devcfg, &spi);
    disp_spi = spi;
    TFT_display_init();
    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);

    font_rotate = 0;
    text_wrap = 0;
    font_transparent = 0;
    font_forceFixed = 0;
    gray_scale = 0;
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
}

int heading_height;

static void draw_screen(void)
{
    TFT_fillScreen(TFT_BLACK);
    TFT_resetclipwin();

    _fg = TFT_YELLOW;
    _bg = (color_t){64, 64, 64};

    TFT_setFont(DEF_SMALL_FONT, NULL);

    TFT_fillRect(0, 0, _width - 1, TFT_getfontheight() + 8, _bg);
    TFT_drawRect(0, 0, _width - 1, TFT_getfontheight() + 8, TFT_CYAN);

    TFT_print("Environment Monitor", CENTER, 4);
    _bg = TFT_BLACK;
    TFT_setclipwin(0, TFT_getfontheight() + 9, _width - 1, _height - TFT_getfontheight() - 10);

    _fg = TFT_WHITE;
    TFT_setFont(SMALL_FONT, NULL);
    heading_height = TFT_getfontheight();
    TFT_print("Temperature", CENTER, _width / 4);
    TFT_print("Humidity", CENTER, _width / 2);
}

void display_reading(int temp, int hum)
{
    char buff[10];

    _fg = TFT_RED;
    TFT_setFont(UBUNTU16_FONT, NULL);
    sprintf(buff, "%d C", temp);
    TFT_print(buff, CENTER, _width / 4 + heading_height + 2);
    sprintf(buff, "%d %%", hum);
    TFT_print(buff, CENTER, _width / 2 + heading_height + 2);
}

void read_dht11(void *arg)
{
    int16_t humidity, temperature;
    while (1)
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