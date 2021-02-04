#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>
#include "driver/uart.h"
#include <stdio.h>

#define UART_PORT UART_NUM_2
#define TXD_PIN 17
#define RXD_PIN 16
#define UART_BUFF_SIZE 1024

static void init_hw(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_PORT, UART_BUFF_SIZE, 0, 0, NULL, 0);
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

static void read_uart(void *arg)
{
    uint8_t buff[UART_BUFF_SIZE];

    while (1)
    {
        if (uart_read_bytes(UART_PORT, buff, UART_BUFF_SIZE, 2000 / portTICK_PERIOD_MS) > 0)
        {
            printf("temp: %d\n", (int)buff[0]);
        }
    }
}

void app_main()
{
    init_hw();

    xTaskCreate(read_uart, "uart", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);
}
