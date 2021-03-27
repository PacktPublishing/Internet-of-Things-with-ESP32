#include <string.h>
#include "app_web.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"

#define TAG "app.web"

static const char *HTML_FORM = "<html><form action=\"/\" method=\"post\">"
                               "<label for=\"switch_state\">Set switch:</label>"
                               "<select id=\"switch_state\" name=\"switch_state\">"
                               "<option value=\"ON\">ON</option>"
                               "<option value=\"OFF\">OFF</option>"
                               "</select>"
                               "<input type=\"submit\" value=\"Submit\">"
                               "</form></html>";

static set_switch_f set_sw = NULL;

void appweb_init(set_switch_f f)
{
    set_sw = f;
}

static esp_err_t handle_http_get(httpd_req_t *req)
{
    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t handle_http_post(httpd_req_t *req)
{
    char content[100];
    if (httpd_req_recv(req, content, req->content_len) <= 0)
    {
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "%.*s", req->content_len, content);
    content[req->content_len] = 0;
    char *val = strchr(content, '=');
    if (set_sw)
    {
        set_sw(!strcmp(val + 1, "ON"));
    }

    return httpd_resp_send(req, HTML_FORM, HTTPD_RESP_USE_STRLEN);
}

void appweb_start_server(void)
{
    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = handle_http_get,
        .user_ctx = NULL};

    httpd_uri_t uri_post = {
        .uri = "/",
        .method = HTTP_POST,
        .handler = handle_http_post,
        .user_ctx = NULL};

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
}
