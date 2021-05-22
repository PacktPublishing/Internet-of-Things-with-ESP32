
#ifndef app_hw_h_
#define app_hw_h_

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"

// GPIO pins
#define APP_BTN0 19 // OFF
#define APP_BTN1 18 // 33%
#define APP_BTN2 5  // 66%
#define APP_BTN3 17 // 100%
#define APP_OE 27   // ENABLE
#define APP_RELAY1 32
#define APP_RELAY2 33
#define APP_RELAY3 25

typedef struct
{
    gpio_num_t btn_pin;
    gpio_num_t relay_pin;
    uint8_t val;
} btn_map_t;

typedef void (*appbtn_fan_changed_f)(uint8_t);

void apphw_init(appbtn_fan_changed_f);
uint8_t apphw_get_state(void);
void apphw_set_state(uint8_t);

#endif