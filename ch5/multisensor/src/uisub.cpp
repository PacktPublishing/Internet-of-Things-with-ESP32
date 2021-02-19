
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "uisub.h"
#include <driver/gpio.h>
#include "rotenc.h"
#include "esp_log.h"
#include <ssd1306.h>

#define APPTAG "msensor"
#define DISP_MAX_X (ssd1306_displayWidth() - 1)
#define DISP_MAX_Y (ssd1306_displayHeight() - 1)

#define SHOW_TEMPERATURE 0
#define SHOW_HUMIDITY 1
#define SHOW_PRESSURE 2
#define SHOW_LUX 3

static int rotenc_pos = 0;
static uisub_config_t config;
static SemaphoreHandle_t disp_guard;
static sensor_reading_t last_reading;

static void rotenc_handler(void *arg);
static void reset_display(void);
static void beep(void *arg);

void uisub_init(uisub_config_t c)
{
    config = c;

    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << config.buzzer_pin);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    rotenc_init(config.rotenc_clk_pin, config.rotenc_dt_pin);
    rotenc_setPosChangedCallback(rotenc_handler);

    ssd1306_128x64_i2c_initEx(config.oled_scl, config.oled_sda, 0);
    disp_guard = xSemaphoreCreateMutex();

    reset_display();
}

void uisub_sleep(void)
{
    ssd1306_displayOff();
}

void uisub_resume(void)
{
    ssd1306_displayOn();
}

void uisub_beep(int cnt)
{
    xTaskCreate(beep, "beep", 3 * configMINIMAL_STACK_SIZE, (void *)cnt, 5, NULL);
}

void uisub_show(sensor_reading_t data)
{
    if (xSemaphoreTake(disp_guard, 0) == pdFALSE)
    {
        return;
    }
    reset_display();
    ssd1306_setFixedFont(ssd1306xled_font8x16);
    char buff[10];

    switch ((rotenc_pos / 4) % 4)
    {
    case SHOW_TEMPERATURE:
        ssd1306_printFixed(0, 5, "Temp", STYLE_NORMAL);
        ssd1306_setFixedFont(ssd1306xled_font6x8);
        sprintf(buff, "%d", data.temperature);
        ssd1306_printFixedN(48, 16, buff, STYLE_BOLD, 2);
        break;
    case SHOW_HUMIDITY:
        ssd1306_printFixed(0, 19, "Hum", STYLE_NORMAL);
        ssd1306_setFixedFont(ssd1306xled_font6x8);
        sprintf(buff, "%d", data.humidity);
        ssd1306_printFixedN(48, 16, buff, STYLE_BOLD, 2);
        break;
    case SHOW_PRESSURE:
        ssd1306_printFixed(0, 33, "Pres", STYLE_NORMAL);
        ssd1306_setFixedFont(ssd1306xled_font6x8);
        sprintf(buff, "%d", data.pressure);
        ssd1306_printFixedN(48, 16, buff, STYLE_BOLD, 2);
        break;
    case SHOW_LUX:
        ssd1306_printFixed(0, 47, "Lux", STYLE_NORMAL);
        ssd1306_setFixedFont(ssd1306xled_font6x8);
        sprintf(buff, "%d", data.light);
        ssd1306_printFixedN(48, 16, buff, STYLE_BOLD, 2);
        break;

    default:
        break;
    }

    last_reading = data;

    xSemaphoreGive(disp_guard);
}

static void reset_display(void)
{
    ssd1306_clearScreen();
    ssd1306_drawLine(0, 0, DISP_MAX_X, 0);
    ssd1306_drawLine(DISP_MAX_X, 1, DISP_MAX_X, DISP_MAX_Y);
    ssd1306_drawLine(DISP_MAX_X - 1, DISP_MAX_Y, 0, DISP_MAX_Y);
    ssd1306_drawLine(0, DISP_MAX_Y - 1, 0, 0);
}

static void rotenc_handler(void *arg)
{
    while (1)
    {
        int pos1 = rotenc_getPos();
        ESP_LOGD(APPTAG, "rotenc_pos: %d", pos1);
        if (abs(pos1 - rotenc_pos) >= 4)
        {
            rotenc_pos = pos1;
            if (config.rotenc_changed != NULL)
            {
                config.rotenc_changed();
            }
            uisub_show(last_reading);
        }
        vTaskSuspend(NULL);
    }
}

static void beep(void *arg)
{
    int cnt = 2 * (int)arg;
    bool state = true;
    for (int i = 0; i < cnt; ++i, state = !state)
    {
        gpio_set_level((gpio_num_t)config.buzzer_pin, state);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}
