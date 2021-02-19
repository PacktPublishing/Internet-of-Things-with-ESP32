#include "wifi_connect.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht.h"
#include "hal/gpio_types.h"
#include "mdns.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#define DHT11_PIN GPIO_NUM_17
#define SVC_PORT 1111

static const char *TAG = "sensor_app";

static int16_t temperature;
static int16_t humidity;

static void start_mdns(void)
{
    mdns_init();
    mdns_hostname_set("esp32_sensor");
    mdns_instance_name_set("esp32 with dht11");

    mdns_txt_item_t serviceTxtData[4] = {
        {"temperature", "y"},
        {"humidity", "y"},
        {"pressure", "n"},
        {"light", "n"},
    };

    mdns_service_add("ESP32-Sensor", "_sensor", "_udp", SVC_PORT, serviceTxtData, 4);
}

static void start_udp_server(void)
{
    char data_buffer[64];
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SVC_PORT),
        .sin_addr = {
            .s_addr = htonl(INADDR_ANY)}};

    while (1)
    {
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            ESP_LOGE(TAG, "bind failed");
        }
        else
        {
            while (1)
            {
                struct sockaddr_storage client_addr;
                socklen_t socklen = sizeof(client_addr);
                int len = recvfrom(sock, data_buffer,
                                   sizeof(data_buffer) - 1, 0,
                                   (struct sockaddr *)&client_addr,
                                   &socklen);

                if (len < 0)
                {
                    ESP_LOGE(TAG, "recvfrom failed");
                    break;
                }
                data_buffer[len] = 0;

                if (!strcmp(data_buffer, "temperature"))
                {
                    sprintf(data_buffer, "%d", temperature);
                }
                else if (!strcmp(data_buffer, "humidity"))
                {
                    sprintf(data_buffer, "%d", humidity);
                }
                else
                {
                    sprintf(data_buffer, "err");
                }

                len = strlen(data_buffer);

                if (sendto(sock, data_buffer, len, 0,
                           (struct sockaddr *)&client_addr,
                           sizeof(client_addr)) < 0)
                {
                    ESP_LOGE(TAG, "sendto failed");
                    break;
                }
            }
        }

        if (sock != -1)
        {
            shutdown(sock, 0);
            close(sock);
        }

        vTaskDelay(1000);
    }
}

static void start_sensor_service(void *arg)
{
    start_mdns();
    start_udp_server();

    vTaskDelete(NULL);
}

static void wifi_connected_cb(void)
{
    ESP_LOGI(TAG, "wifi connected");
    xTaskCreate(start_sensor_service, "svc", 5 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}

static void wifi_failed_cb(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

static void read_dht11(void *arg)
{
    while (1)
    {
        vTaskDelay(2000 / portTICK_RATE_MS);
        dht_read_data(DHT_TYPE_DHT11, DHT11_PIN, &humidity, &temperature);
        humidity /= 10;
        temperature /= 10;
    }
}

void app_main()
{
    connect_wifi_params_t p = {
        .on_connected = wifi_connected_cb,
        .on_failed = wifi_failed_cb};
    connect_wifi(p);
    xTaskCreate(read_dht11, "temp", 3 * configMINIMAL_STACK_SIZE, NULL, 5, NULL);
}