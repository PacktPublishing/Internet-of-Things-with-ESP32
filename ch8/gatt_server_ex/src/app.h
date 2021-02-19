

#ifndef gattex_app_h_
#define gattex_app_h_

#include <stdint.h>
#include <stddef.h>
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

#define GATT_SERVICE_UUID 0x00FF
#define GATT_CHARACTERISTIC_UUID 0xFF01
#define GATT_HANDLE_COUNT 4

typedef struct
{
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
    esp_gatt_if_t gatts_if;
    uint16_t client_write_conn;

} service_info_t;

extern esp_ble_adv_data_t adv_data;
extern esp_ble_adv_params_t adv_params;
extern service_info_t service_def;

void init_service_def(void);
void update_conn_params(esp_bd_addr_t remote_bda);

#endif