#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "board.h"

#define TAG "app.board"

static board_state_changed_f board_state_changed_cb;
static uint8_t led_state = LED_OFF;

void board_set_led(uint8_t onoff)
{
    led_state = onoff;
    gpio_set_level(GPIO_LED, led_state);
}

static TickType_t next = 0;

static void run_board_state_changed(void *arg)
{
    if (board_state_changed_cb != NULL)
    {
        board_state_changed_cb(led_state);
    }
    vTaskDelete(NULL);
}

static void IRAM_ATTR button_handler(void *arg)
{
    TickType_t now = xTaskGetTickCountFromISR();
    if (now > next)
    {
        led_state = !led_state;
        gpio_set_level(GPIO_LED, led_state);
        next = now + 500 / portTICK_PERIOD_MS;
        xTaskCreate(run_board_state_changed, "cb", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
    }
}

void board_init(board_state_changed_f cb)
{
    board_state_changed_cb = cb;

    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_LED);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_BUTTON);
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_BUTTON, button_handler, NULL);
}
