#include <esp_system.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#include "esp_camera.h"

#define PIR_MOTION_PIN 3

static void pir_handler(void *arg);
static void take_pic(void *arg);

static esp_err_t init_hw(void)
{
    camera_config_t camera_config = {
        .pin_pwdn = CONFIG_PWDN,
        .pin_reset = CONFIG_RESET,
        .pin_xclk = CONFIG_XCLK,
        .pin_sscb_sda = CONFIG_SDA,
        .pin_sscb_scl = CONFIG_SCL,

        .pin_d7 = CONFIG_D7,
        .pin_d6 = CONFIG_D6,
        .pin_d5 = CONFIG_D5,
        .pin_d4 = CONFIG_D4,
        .pin_d3 = CONFIG_D3,
        .pin_d2 = CONFIG_D2,
        .pin_d1 = CONFIG_D1,
        .pin_d0 = CONFIG_D0,
        .pin_vsync = CONFIG_VSYNC,
        .pin_href = CONFIG_HREF,
        .pin_pclk = CONFIG_PCLK,

        .xclk_freq_hz = CONFIG_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_UXGA,

        .jpeg_quality = 12,
        .fb_count = 1};
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        return err;
    }

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
    };
    sdmmc_card_t *card;
    err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (err != ESP_OK)
    {
        return err;
    }

    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIR_MOTION_PIN);
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.pull_up_en = 1;
    err = gpio_config(&io_conf);
    if (err != ESP_OK)
    {
        return err;
    }
    return gpio_isr_handler_add(PIR_MOTION_PIN, pir_handler, NULL);
}

static TickType_t next = 0;
const TickType_t period = 20000 / portTICK_PERIOD_MS;

static void IRAM_ATTR pir_handler(void *arg)
{
    TickType_t now = xTaskGetTickCountFromISR();

    if (now > next)
    {
        xTaskCreate(take_pic, "pic", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
    }
    next = now + period;
}

static void take_pic(void *arg)
{
    printf("Say cheese!\n");

    camera_fb_t *pic = esp_camera_fb_get();

    char pic_name[50];
    sprintf(pic_name, "/sdcard/pic_%li.jpg", pic->timestamp.tv_sec);

    FILE *file = fopen(pic_name, "w");
    if (file == NULL)
    {
        printf("err: fopen failed\n");
    }
    else
    {
        fwrite(pic->buf, 1, pic->len, file);
        fclose(file);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    esp_err_t err;
    err = init_hw();
    if (err != ESP_OK)
    {
        printf("err: %s\n", esp_err_to_name(err));
        return;
    }
}