#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

#include "esp_log.h"

#include "app_temp.h"
#include "app_wifi.h"

#include "esp_spiffs.h"
#include "esp_http_server.h"

const static char *TAG = "app";

#define INDEX_HTML_PATH "/spiffs/index.html"
static char index_html[4096];

static int temperature, humidity;
static bool enabled = true;

static httpd_handle_t server = NULL;
static int ws_fd = -1;

static void init_html(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};

    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    memset((void *)index_html, 0, sizeof(index_html));
    struct stat st;
    if (stat(INDEX_HTML_PATH, &st))
    {
        ESP_LOGE(TAG, "index.html not found");
        return;
    }

    FILE *fp = fopen(INDEX_HTML_PATH, "r");
    if (fread(index_html, st.st_size, 1, fp) != st.st_size)
    {
        ESP_LOGE(TAG, "fread failed");
    }
    fclose(fp);
}

static void send_async(void *arg)
{
    if (ws_fd < 0)
    {
        return;
    }

    char buff[128];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "{\"state\": \"%s\", \"temp\": %d, \"hum\": %d}",
            enabled ? "ON" : "OFF", temperature, humidity);
    
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)buff;
    ws_pkt.len = strlen(buff);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(server, ws_fd, &ws_pkt);
}

static esp_err_t handle_http_get(httpd_req_t *req)
{
    if (index_html[0] == 0)
    {
        httpd_resp_set_status(req, HTTPD_500);
        return httpd_resp_send(req, "no index.html", HTTPD_RESP_USE_STRLEN);
    }
    return httpd_resp_send(req, index_html, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t handle_ws_req(httpd_req_t *req)
{
    enabled = !enabled;

    httpd_ws_frame_t ws_pkt;
    uint8_t buff[16];
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = buff;
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;

    httpd_ws_recv_frame(req, &ws_pkt, sizeof(buff));

    if (!enabled)
    {
        httpd_queue_work(server, send_async, NULL);
    }
    return ESP_OK;
}

static esp_err_t handle_socket_opened(httpd_handle_t hd, int sockfd)
{
    ws_fd = sockfd;
    return ESP_OK;
}

static void handle_socket_closed(httpd_handle_t hd, int sockfd)
{
    if (sockfd == ws_fd)
    {
        ws_fd = -1;
    }
}

static void start_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.open_fn = handle_socket_opened;
    config.close_fn = handle_socket_closed;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t uri_get = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = handle_http_get,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &uri_get);

        httpd_uri_t ws = {
            .uri = "/ws",
            .method = HTTP_GET,
            .handler = handle_ws_req,
            .user_ctx = NULL,
            .is_websocket = true};
        httpd_register_uri_handler(server, &ws);
    }
}

static void update_reading(int temp, int hum)
{
    temperature = temp;
    humidity = hum;

    if (server != NULL && enabled)
    {
        httpd_queue_work(server, send_async, NULL);
    }
}

static void handle_wifi_connect(void)
{
    start_server();
    apptemp_init(update_reading);
}

static void handle_wifi_failed(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main()
{
    init_html();

    connect_wifi_params_t cbs = {
        .on_connected = handle_wifi_connect,
        .on_failed = handle_wifi_failed};
    appwifi_connect(cbs);
}