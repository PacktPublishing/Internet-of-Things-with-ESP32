/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef app_blecommon_h_
#define app_blecommon_h_

#include <stdint.h>

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_health_model_api.h"


void appble_get_dev_uuid(uint8_t *dev_uuid);

esp_err_t appble_bt_init(void);

void appble_provisioning_handler(esp_ble_mesh_prov_cb_event_t event,
                                 esp_ble_mesh_prov_cb_param_t *param);

void appble_config_handler(esp_ble_mesh_cfg_server_cb_event_t event,
                           esp_ble_mesh_cfg_server_cb_param_t *param);

void appble_health_evt_handler(esp_ble_mesh_health_server_cb_event_t event,
                               esp_ble_mesh_health_server_cb_param_t *param);

typedef void (*appble_attn_on_f)(void);
typedef void (*appble_attn_off_f)(void);
typedef struct
{
   appble_attn_on_f attn_on;
   appble_attn_off_f attn_off;
} appble_attn_cbs_t;

void appble_set_attn_cbs(appble_attn_cbs_t);

#endif
