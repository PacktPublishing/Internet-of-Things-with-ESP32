
#include "app_ble.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_gap_ble_api.h"
#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"

#define TAG "app.ble"

#define CID_ESP 0x02E5

static uint8_t dev_uuid[16] = {0xdd, 0xdd};

static esp_ble_mesh_client_t onoff_client;

static esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_DISABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
    .default_ttl = 7,
    .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

ESP_BLE_MESH_MODEL_PUB_DEFINE(onoff_cli_pub, 2 + 1, ROLE_NODE);

static uint8_t test_ids[] = {0, 1, 2};
ESP_BLE_MESH_MODEL_PUB_DEFINE(health_pub_0, 10, ROLE_NODE);
static esp_ble_mesh_health_srv_t health_server = {
    .health_test = {
        .id_count = ARRAY_SIZE(test_ids),
        .test_ids = test_ids,
        .company_id = CID_ESP,
    },
};

static esp_ble_mesh_model_t root_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_MODEL_GEN_ONOFF_CLI(&onoff_cli_pub, &onoff_client),
    ESP_BLE_MESH_MODEL_HEALTH_SRV(&health_server, &health_pub_0),
};

static esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, root_models, ESP_BLE_MESH_MODEL_NONE),
};

static esp_ble_mesh_comp_t composition = {
    .cid = CID_ESP,
    .elements = elements,
    .element_count = ARRAY_SIZE(elements),
};

static esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
    .output_size = 0,
    .output_actions = 0,
};

void appble_set_switch(bool onoff)
{
    esp_ble_mesh_generic_client_set_state_t set = {0};
    esp_ble_mesh_client_common_param_t common = {0};

    common.opcode = ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK;
    common.model = onoff_client.model;
    common.ctx.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    common.ctx.app_idx = 0;
    common.ctx.addr = 0xFFFF;
    common.ctx.send_ttl = 3;
    common.ctx.send_rel = false;
    common.msg_timeout = 0;
    common.msg_role = ROLE_NODE;

    set.onoff_set.op_en = false;
    set.onoff_set.onoff = onoff;
    set.onoff_set.tid = 0;

    if (esp_ble_mesh_generic_client_set_state(&common, &set) != ESP_OK)
    {
        ESP_LOGE(TAG, "appble_set_switch failed");
        return;
    }
}

static void generic_client_cb(esp_ble_mesh_generic_client_cb_event_t event,
                              esp_ble_mesh_generic_client_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT:
        ESP_LOGI(TAG, "ESP_BLE_MESH_GENERIC_CLIENT_TIMEOUT_EVT");
        break;
    default:
        break;
    }
}

void init_ble(appble_attn_cbs_t cbs)
{
    appble_set_attn_cbs(cbs);

    if (appble_bt_init() != ESP_OK)
    {
        ESP_LOGE(TAG, "bluetooth_init failed");
        return;
    }

    appble_get_dev_uuid(dev_uuid);

    esp_ble_mesh_register_prov_callback(appble_provisioning_handler);
    esp_ble_mesh_register_config_server_callback(appble_config_handler);
    esp_ble_mesh_register_health_server_callback(appble_health_evt_handler);

    esp_ble_mesh_register_generic_client_callback(generic_client_cb);

    if (esp_ble_mesh_init(&provision, &composition) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize mesh stack");
        return;
    }
    if (esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable mesh node");
        return;
    }
}