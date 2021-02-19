
#ifndef appmesh_setup_h_
#define appmesh_setup_h_

#include "esp_ble_mesh_defs.h"
#include "esp_ble_mesh_generic_model_api.h"
#include "esp_err.h"


esp_ble_mesh_model_t *appmesh_get_onoff_model(void);
esp_ble_mesh_gen_onoff_srv_t *appmesh_get_onoff_server(void);

esp_err_t appmesh_init(void);

#endif // !appmesh_setup_h_
