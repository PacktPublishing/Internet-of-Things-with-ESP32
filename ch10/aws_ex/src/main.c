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

#define TAG "app"

#define SENSOR_NO "1"
#define ENABLE_TOPIC "home/" SENSOR_NO "/enable"
#define TEMP_TOPIC "home/temperature/" SENSOR_NO
#define HUM_TOPIC "home/humidity/" SENSOR_NO

extern const uint8_t aws_root_ca_pem_start[] asm("_binary_AmazonRootCA1_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_AmazonRootCA1_pem_end");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_my_sensor1_cert_pem_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_my_sensor1_cert_pem_end");
extern const uint8_t private_pem_key_start[] asm("_binary_my_sensor1_private_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_my_sensor1_private_key_end");

static char endpoint_address[] = AWS_ENDPOINT;
static char client_id[] = "my_sensor1";

static AWS_IoT_Client aws_client;
static bool enabled = false;

static void publish_reading(int temp, int hum)
{
    IoT_Error_t res = aws_iot_mqtt_yield(&aws_client, 100);
    if (res != SUCCESS && res != NETWORK_RECONNECTED)
    {
        return;
    }
    if (!enabled)
    {
        return;
    }

    char buffer[5];
    IoT_Publish_Message_Params message;
    memset((void *)&message, 0, sizeof(message));

    itoa(temp, buffer, 10);
    message.qos = QOS0;
    message.payload = (void *)buffer;
    message.payloadLen = strlen(buffer);
    aws_iot_mqtt_publish(&aws_client, TEMP_TOPIC, strlen(TEMP_TOPIC), &message);

    itoa(hum, buffer, 10);
    message.payloadLen = strlen(buffer);
    aws_iot_mqtt_publish(&aws_client, HUM_TOPIC, strlen(HUM_TOPIC), &message);
}

void subscribe_handler(AWS_IoT_Client *pClient,
                       char *topicName, uint16_t topicNameLen,
                       IoT_Publish_Message_Params *params,
                       void *pData)
{
    enabled = ((char *)params->payload)[0] - '0';
}

void disconnected_handler(AWS_IoT_Client *pClient, void *data)
{
    ESP_LOGW(TAG, "reconnecting...");
}

void connect_aws_mqtt(void *param)
{
    memset((void *)&aws_client, 0, sizeof(aws_client));

    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    mqttInitParams.pHostURL = endpoint_address;
    mqttInitParams.port = AWS_IOT_MQTT_PORT;
    mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
    mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
    mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;
    mqttInitParams.disconnectHandler = disconnected_handler;
    aws_iot_mqtt_init(&aws_client, &mqttInitParams);

    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;
    connectParams.keepAliveIntervalInSec = 10;
    connectParams.pClientID = client_id;
    connectParams.clientIDLen = (uint16_t)strlen(client_id);
    while (aws_iot_mqtt_connect(&aws_client, &connectParams) != SUCCESS)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    ESP_LOGI(TAG, "connected");

    aws_iot_mqtt_subscribe(&aws_client, ENABLE_TOPIC, strlen(ENABLE_TOPIC), QOS0, subscribe_handler, NULL);

    while (1)
    {
        aws_iot_mqtt_yield(&aws_client, 100);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void handle_wifi_connect(void)
{
    xTaskCreate(connect_aws_mqtt, "connect_aws_mqtt", 15 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
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