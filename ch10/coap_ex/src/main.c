#include <string.h>
#include <stdint.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_event.h"

#include "app_temp.h"
#include "app_wifi.h"

#include "coap.h"

const static char *TAG = "app";

static int temperature, humidity;

static void
handle_sensor_get(coap_context_t *ctx,
                  coap_resource_t *resource,
                  coap_session_t *session,
                  coap_pdu_t *request,
                  coap_binary_t *token,
                  coap_string_t *query,
                  coap_pdu_t *response)
{
    char buff[100];
    memset(buff, 0, sizeof(buff));
    if (!strcmp("temperature", (const char *)(resource->uri_path->s)))
    {
        sprintf(buff, "{\"temperature\": %d}", temperature);
    }
    else
    {
        sprintf(buff, "{\"humidity\": %d}", humidity);
    }
    coap_add_data_blocked_response(resource, session, request, response, token,
                                   COAP_MEDIATYPE_APPLICATION_JSON, 0,
                                   strlen(buff), (const uint8_t *)buff);
}

static void sensor_server(void *p)
{
    coap_context_t *ctx = NULL;
    coap_address_t serv_addr;
    coap_resource_t *temp_resource = NULL;
    coap_resource_t *hum_resource = NULL;

    coap_set_log_level(LOG_DEBUG);

    while (1)
    {
        coap_address_init(&serv_addr);
        serv_addr.addr.sin.sin_family = AF_INET;
        serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
        serv_addr.addr.sin.sin_port = htons(COAP_DEFAULT_PORT);

        ctx = coap_new_context(NULL);
        coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);

        temp_resource = coap_resource_init(coap_make_str_const("temperature"), 0);
        coap_register_handler(temp_resource, COAP_REQUEST_GET, handle_sensor_get);
        coap_add_resource(ctx, temp_resource);

        hum_resource = coap_resource_init(coap_make_str_const("humidity"), 0);
        coap_register_handler(hum_resource, COAP_REQUEST_GET, handle_sensor_get);
        coap_add_resource(ctx, hum_resource);

        while (1)
        {
            int result = coap_run_once(ctx, 2000);
            if (result < 0)
            {
                break;
            }
        }
        coap_free_context(ctx);
        coap_cleanup();
    }

    vTaskDelete(NULL);
}

static void update_reading(int temp, int hum)
{
    temperature = temp;
    humidity = hum;
}

static void handle_wifi_connect(void)
{
    xTaskCreate(sensor_server, "coap", 8 * 1024, NULL, 5, NULL);
    apptemp_init(update_reading);
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