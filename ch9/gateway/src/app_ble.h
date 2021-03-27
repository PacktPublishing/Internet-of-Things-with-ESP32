
#ifndef app_ble_h_
#define app_ble_h_

#include <stdlib.h>
#include <stdbool.h>
#include "app_blecommon.h"


void init_ble(appble_attn_cbs_t);
void appble_set_switch(bool);

#endif