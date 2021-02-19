
#ifndef uisub_h_
#define uisub_h_

#include "common.h"

typedef void (*rotenc_changed_f)(void);

typedef struct
{
    int buzzer_pin;

    int rotenc_clk_pin;
    int rotenc_dt_pin;
    rotenc_changed_f rotenc_changed;

    int oled_sda;
    int oled_scl;
} uisub_config_t;

extern "C"
{
    void uisub_init(uisub_config_t);
    void uisub_sleep(void);
    void uisub_resume(void);
    void uisub_beep(int);
    void uisub_show(sensor_reading_t);
}

#endif