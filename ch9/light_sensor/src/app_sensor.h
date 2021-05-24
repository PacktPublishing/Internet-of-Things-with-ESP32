
#ifndef app_sensor_h_
#define app_sensor_h_

#include "tsl2561.h"

#define LIGHT_THRESHOLD 30
#define LIGHT_SDA 21
#define LIGHT_SCL 22
#define LIGHT_ADDR TSL2561_I2C_ADDR_FLOAT


typedef void (*light_changed_f)(bool);
void init_hw(light_changed_f);
bool is_light_low(void);

#endif