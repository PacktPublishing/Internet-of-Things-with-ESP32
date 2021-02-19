#ifndef _BOARD_H_
#define _BOARD_H_

#include "driver/gpio.h"

#define LED_ON  1
#define LED_OFF 0

typedef void (*board_state_changed_f)(uint8_t);


void board_init(board_state_changed_f);
void board_set_led(uint8_t onoff);


#define GPIO_LED 2
#define GPIO_BUTTON 5


#endif
