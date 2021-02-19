#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"

#include "board.h"
#include "appmesh_setup.h"
#include "appbt_init.h"

#define TAG "app"

static void provisioning_cb(esp_ble_mesh_prov_cb_event_t event,
                            esp_ble_mesh_prov_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BLE_MESH_NODE_PROV_COMPLETE_EVT:
        ESP_LOGI(TAG, "provisioned. addr: 0x%04x", param->node_prov_complete.addr);
        break;
    case ESP_BLE_MESH_NODE_PROV_RESET_EVT:
        ESP_LOGI(TAG, "node reset");
        esp_ble_mesh_node_local_reset();
        break;
    default:
        break;
    }
}

static void generic_server_cb(esp_ble_mesh_generic_server_cb_event_t event,
                              esp_ble_mesh_generic_server_cb_param_t *param)
{
    switch (event)
    {
    case ESP_BLE_MESH_GENERIC_SERVER_STATE_CHANGE_EVT:
        ESP_LOGI(TAG, "event name: state-changed");
        if (param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET ||
            param->ctx.recv_op == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_SET_UNACK)
        {
            board_set_led(param->value.state_change.onoff_set.onoff);
        }
        break;
    default:
        ESP_LOGW(TAG, "unexpected event");
        break;
    }
}

static void config_server_cb(esp_ble_mesh_cfg_server_cb_event_t event,
                             esp_ble_mesh_cfg_server_cb_param_t *param)
{
    if (event == ESP_BLE_MESH_CFG_SERVER_STATE_CHANGE_EVT)
    {
        switch (param->ctx.recv_op)
        {
        case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD:
            ESP_LOGI(TAG, "config: app key added");
            break;
        default:
            break;
        }
    }
}

static void update_element_cb(uint8_t led_s)
{
    esp_ble_mesh_model_t *model = appmesh_get_onoff_model();
    esp_ble_mesh_gen_onoff_srv_t *srv = appmesh_get_onoff_server();
    srv->state.onoff = led_s;

    esp_ble_mesh_model_publish(model, ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_STATUS,
                               sizeof(srv->state.onoff), &srv->state.onoff, ROLE_NODE);
}

void app_main(void)
{
    esp_err_t err;

    board_init(update_element_cb);
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(appbt_init());

    esp_ble_mesh_register_prov_callback(provisioning_cb);
    esp_ble_mesh_register_config_server_callback(config_server_cb);
    esp_ble_mesh_register_generic_server_callback(generic_server_cb);
    ESP_ERROR_CHECK(appmesh_init());
}
