
#include "app.h"
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include "esp_spiffs.h"
#include "esp_err.h"
#include "driver/i2s.h"
#include "hal/i2s_types.h"

#define BCLK_PIN 25
#define LRC_PIN 26
#define DIN_PIN 22

static const int i2s_num = I2S_NUM_0;
static uint8_t buff[1024];
static FILE *wav_fp;

static esp_err_t init_hw(void)
{
    printf("Initializing SPIFFS\n");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    return esp_vfs_spiffs_register(&conf);
}

static esp_err_t open_file(wav_header_t *header)
{
    wav_fp = fopen("/spiffs/rooster.wav", "rb");
    if (wav_fp == NULL)
    {
        printf("err: no file\n");
        return ESP_ERR_INVALID_ARG;
    }

    fread((void *)header, sizeof(wav_header_t), 1, wav_fp);
    printf("Wav format:\n");
    printf("bit_depth: %d\n", header->bit_depth);
    printf("num_channels: %d\n", header->num_channels);
    printf("sample_rate: %d\n", header->sample_rate);

    return ESP_OK;
}

static esp_err_t init_i2s(wav_header_t *header)
{
    esp_err_t err;

    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = header->sample_rate,
        .bits_per_sample = header->bit_depth,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .channel_format = header->num_channels == 2 ? I2S_CHANNEL_FMT_RIGHT_LEFT : I2S_CHANNEL_FMT_ONLY_LEFT,
        .intr_alloc_flags = 0,
        .dma_buf_count = 2,
        .dma_buf_len = 1024,
        .use_apll = 1,
    };
    err = i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    if (err != ESP_OK)
    {
        return err;
    }

    i2s_pin_config_t pin_config = {
        .bck_io_num = BCLK_PIN,
        .ws_io_num = LRC_PIN,
        .data_out_num = DIN_PIN,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };
    err = i2s_set_pin(i2s_num, &pin_config);
    if (err != ESP_OK)
    {
        return err;
    }

    return i2s_zero_dma_buffer(i2s_num);
}

void app_main(void)
{
    esp_err_t ret;

    ret = init_hw();
    if (ret != ESP_OK)
    {
        printf("err: %s\n", esp_err_to_name(ret));
        return;
    }

    wav_header_t header;
    ret = open_file(&header);
    if (ret != ESP_OK)
    {
        printf("err: %s\n", esp_err_to_name(ret));
        return;
    }

    ret = init_i2s(&header);
    if (ret != ESP_OK)
    {
        printf("err: %s\n", esp_err_to_name(ret));
        return;
    }

    size_t bytes_written;
    size_t cnt;

    while (1)
    {
        cnt = fread(buff, 1, sizeof(buff), wav_fp);
        ret = i2s_write(i2s_num, (const void *)buff, sizeof(buff), &bytes_written, portMAX_DELAY);
        if (ret != ESP_OK)
        {
            printf("err: %s\n", esp_err_to_name(ret));
            break;
        }
        if (cnt < sizeof(buff))
        {
            break;
        }
    }

    fclose(wav_fp);
    i2s_driver_uninstall(i2s_num);
}