#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "app_temp.h"
#include "app_wifi.h"

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#define TAG "app"

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_AmazonRootCA1_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_AmazonRootCA1_pem_end");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

static char endpoint_address[] = AWS_ENDPOINT;
static char client_id[] = "myhome_sensor1_cl";
static char thing_name[] = "myhome_sensor1";

static AWS_IoT_Client aws_client;

static void publish_reading(int temp, int hum)
{
    jsonStruct_t temp_json = {
        .cb = NULL,
        .pKey = "temperature",
        .pData = &temp,
        .type = SHADOW_JSON_INT32,
        .dataLength = sizeof(temp)};

    char jsondoc_buffer[200];
    aws_iot_shadow_init_json_document(jsondoc_buffer, sizeof(jsondoc_buffer));
    aws_iot_shadow_add_reported(jsondoc_buffer, sizeof(jsondoc_buffer), 1, &temp_json);
    aws_iot_finalize_json_document(jsondoc_buffer, sizeof(jsondoc_buffer));

    aws_iot_shadow_update(&aws_client, thing_name, jsondoc_buffer,
                          NULL, NULL, 4, true);
}

static void disconnected_handler(AWS_IoT_Client *pClient, void *data)
{
    ESP_LOGW(TAG, "reconnecting...");
}

static void connect_shadow(void *param)
{
    memset((void *)&aws_client, 0, sizeof(aws_client));

    ShadowInitParameters_t sp = ShadowInitParametersDefault;
    sp.pHost = endpoint_address;
    sp.port = AWS_IOT_MQTT_PORT;
    sp.pClientCRT = (const char *)certificate_pem_crt_start;
    sp.pClientKey = (const char *)private_pem_key_start;
    sp.pRootCA = (const char *)aws_root_ca_pem_start;
    sp.disconnectHandler = disconnected_handler;

    aws_iot_shadow_init(&aws_client, &sp);

    ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
    scp.pMyThingName = thing_name;
    scp.pMqttClientId = client_id;
    scp.mqttClientIdLen = (uint16_t)strlen(client_id);

    while (aws_iot_shadow_connect(&aws_client, &scp) != SUCCESS)
    {
        ESP_LOGW(TAG, "trying to connect");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    while (1)
    {
        aws_iot_shadow_yield(&aws_client, 100);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void handle_wifi_connect(void)
{
    xTaskCreate(connect_shadow, "connect_shadow", 15 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    apptemp_init(publish_reading);
}

static void handle_wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main()
{
    connect_wifi_params_t cbs = {
        .on_connected = handle_wifi_connect,
        .on_failed = handle_wifi_failed};
    appwifi_connect(cbs);
}