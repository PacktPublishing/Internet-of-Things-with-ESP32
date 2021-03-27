
#ifndef app_ble_h_
#define app_ble_h_

#include <stdlib.h>
#include <stdbool.h>
#include "app_blecommon.h"

typedef void (*switch_set_f)(bool);
typedef bool (*switch_get_f)(void);

typedef struct {
    switch_set_f sw_set;
    switch_get_f sw_get;
    appble_attn_on_f attn_on;
    appble_attn_off_f attn_off;
} app_ble_cb_t;


void init_ble(app_ble_cb_t callbacks);

#endif