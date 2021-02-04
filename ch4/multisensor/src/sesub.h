
#ifndef sesub_h_
#define sesub_h_

#include "common.h"

typedef void (*sensor_reading_f)(sensor_reading_t);
typedef void (*temp_alarm_f)(void);

typedef struct
{
    int sensor_sda;
    int sensor_scl;

    float temp_high;
    float temp_low;

    sensor_reading_f new_sensor_reading;
    temp_alarm_f temp_alarm;
} sesub_config_t;

extern "C"
{
    void sesub_init(sesub_config_t);
    void sesub_start(void);
}

#endif