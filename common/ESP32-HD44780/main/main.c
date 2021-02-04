#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "HD44780.h"

#define LCD_ADDR 0x27
#define SDA_PIN  19
#define SCL_PIN  18
#define LCD_COLS 20
#define LCD_ROWS 4

static char tag[] = "LCD test";
void LCD_DemoTask(void* param);

void app_main(void)
{
    ESP_LOGI(tag, "Starting up application");
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    xTaskCreate(&LCD_DemoTask, "Demo Task", 2048, NULL, 5, NULL);
}

void LCD_DemoTask(void* param)
{
    char txtBuf[8];
    while (true) {
        int row = 0, col = 0;
        LCD_home();
        LCD_clearScreen();
        LCD_writeStr("----- 20x4 LCD -----");
        LCD_setCursor(0, 1);
        LCD_writeStr("LCD Library Demo");
        LCD_setCursor(12, 3);
        LCD_writeStr("Time: ");
        for (int i = 10; i >= 0; i--) {
            LCD_setCursor(18, 3);
            sprintf(txtBuf, "%02d", i);
            printf(txtBuf);
            LCD_writeStr(txtBuf);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }

        for (int i = 0; i < 80; i++) {
            LCD_clearScreen();
            LCD_setCursor(col, row);
            LCD_writeChar('*');

            if (i >= 19) {
                row = (i + 1) / 20;
            }
            if (col++ >= 19) {
                col = 0;
            }

            vTaskDelay(50 / portTICK_RATE_MS);
        }
    }
}