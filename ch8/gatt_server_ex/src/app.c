
#include "app.h"
#include <string.h>

static uint8_t adv_service_uuid128[32] = {
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

service_info_t service_def;

void init_service_def(void)
{
    service_def.service_id.is_primary = true;
    service_def.service_id.id.inst_id = 0x00;
    service_def.service_id.id.uuid.len = ESP_UUID_LEN_16;
    service_def.service_id.id.uuid.uuid.uuid16 = GATT_SERVICE_UUID;

    service_def.char_uuid.len = ESP_UUID_LEN_16;
    service_def.char_uuid.uuid.uuid16 = GATT_CHARACTERISTIC_UUID;

    service_def.descr_uuid.len = ESP_UUID_LEN_16;
    service_def.descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

    service_def.gatts_if = 0;
}

void update_conn_params(esp_bd_addr_t remote_bda)
{
    esp_ble_conn_update_params_t conn_params = {0};
    memcpy(conn_params.bda, remote_bda, sizeof(esp_bd_addr_t));
    conn_params.latency = 0;
    conn_params.max_int = 0x20;
    conn_params.min_int = 0x10;
    conn_params.timeout = 400;
    esp_ble_gap_update_conn_params(&conn_params);
}