
#include <stdio.h>
#include <string.h>
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"

#include "esp32/ulp.h"
#include "ulp_main.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

static RTC_DATA_ATTR int cnt1 = 0;
static int cnt2 = 0;

static void init_hw()
{
    ulp_load_binary(0, ulp_main_bin_start,
                    (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));

    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_ulp_enable();

    ulp_high_thr = 2000;

    ulp_set_wakeup_period(0, 20000);

    rtc_gpio_isolate(GPIO_NUM_12);
    rtc_gpio_isolate(GPIO_NUM_15);
    esp_deep_sleep_disable_rom_logging();
}

void app_main()
{
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_ULP)
    {
        printf("Powered\n");
        init_hw();
    }
    else
    {
        printf("Wakeup (%d - %d)\n", ++cnt1, ++cnt2);
    }
    ulp_run(&ulp_entry - RTC_SLOW_MEM);
    esp_sleep_enable_ulp_wakeup();
    esp_deep_sleep_start();
}
