// https://github.com/Infineon/mbedtls-optiga-trust-x/wiki
// openssl s_server -tls1_2 -cipher ECDHE-ECDSA-AES128-CCM8 -accept 50000 -cert OPTIGA_Trust_X_InfineonTestServer_EndEntity.pem -key OPTIGA_Trust_X_InfineonTestServer_EndEntity_Key.pem -CAfile OPTIGA_Trust_X_trusted_CAs.pem -debug -state

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_system.h"

#include "mbedtls/platform.h"
#include "mbedtls/net.h"
#include "mbedtls/esp_debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/base64.h"

#include "optiga/optiga_util.h"
#include "optiga/pal/pal_os_event.h"
#include "optiga/ifx_i2c/ifx_i2c_config.h"
#include "wifi_connect.h"
#include "driver/gpio.h"

#define CERT_LENGTH 512
#define WEB_COMMONNAME "Infineon Test Server End Entity Certificate"
#define WEB_PORT "50000"

static const char *TAG = "optiga_ex";

static const char *REQUEST = ">> Super important data from the client |";

optiga_comms_t optiga_comms = {(void *)&ifx_i2c_context_0, NULL, NULL, OPTIGA_COMMS_SUCCESS};

extern const uint8_t server_root_cert_pem_start[] asm("_binary_test_ca_list_pem_start");
extern const uint8_t server_root_cert_pem_end[] asm("_binary_test_ca_list_pem_end");

extern const uint8_t my_key_pem_start[] asm("_binary_dummy_private_key_pem_start");
extern const uint8_t my_key_pem_end[] asm("_binary_dummy_private_key_pem_end");

uint8_t my_cert[CERT_LENGTH];
uint16_t my_cert_len = CERT_LENGTH;

static void exit_task(const char *mesg)
{
    if (mesg)
    {
        ESP_LOGI(TAG, "%s", mesg);
    }
    vTaskDelete(NULL);
}

static void free_mbedtls(mbedtls_ssl_context *ssl, mbedtls_net_context *server_fd, int ret)
{
    char buf[128];
    mbedtls_ssl_session_reset(ssl);
    mbedtls_net_free(server_fd);

    if (ret != 0)
    {
        mbedtls_strerror(ret, buf, sizeof(buf) - 1);
        ESP_LOGE(TAG, "Last error was: -0x%x - %s", -ret, buf);
    }
}

static void connect_server_task(void *args)
{
    int ret = 0;

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt mycert;
    mbedtls_pk_context mykey;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_x509_crt_init(&mycert);
    mbedtls_pk_init(&mykey);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ssl_config_init(&conf);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0);

    if (mbedtls_x509_crt_parse(&cacert, server_root_cert_pem_start,
                               server_root_cert_pem_end - server_root_cert_pem_start) < 0)
    {
        exit_task("mbedtls_x509_crt_parse/root cert failed");
        return;
    }

    if (mbedtls_x509_crt_parse_der(&mycert, my_cert, my_cert_len) < 0)
    {
        exit_task("mbedtls_x509_crt_parse/local cert failed");
        return;
    }

    mbedtls_pk_parse_key(&mykey, (const unsigned char *)my_key_pem_start,
                         my_key_pem_end - my_key_pem_start, NULL, 0);
    mbedtls_ssl_set_hostname(&ssl, WEB_COMMONNAME);
    mbedtls_ssl_config_defaults(&conf,
                                MBEDTLS_SSL_IS_CLIENT,
                                MBEDTLS_SSL_TRANSPORT_STREAM,
                                MBEDTLS_SSL_PRESET_DEFAULT);
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_own_cert(&conf, &mycert, &mykey);
    mbedtls_ssl_setup(&ssl, &conf);

    mbedtls_net_init(&server_fd);
    if ((ret = mbedtls_net_connect(&server_fd, WEB_SERVER, WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        exit_task("mbedtls_net_connect failed");
        free_mbedtls(&ssl, &server_fd, ret);
        return;
    }

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            exit_task("mbedtls_ssl_handshake failed");
            free_mbedtls(&ssl, &server_fd, ret);
            return;
        }
    }

    if ((ret = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        exit_task("mbedtls_ssl_get_verify_result failed");
        free_mbedtls(&ssl, &server_fd, ret);
        return;
    }

    while ((ret = mbedtls_ssl_write(&ssl, (const unsigned char *)REQUEST, strlen(REQUEST))) <= 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            exit_task("mbedtls_ssl_write failed");
            free_mbedtls(&ssl, &server_fd, ret);
            return;
        }
    }

    mbedtls_ssl_close_notify(&ssl);
    free_mbedtls(&ssl, &server_fd, 0);
    exit_task(NULL);
}

static void init_hw(void)
{
    uint8_t err_code;

    gpio_set_direction(CONFIG_PAL_I2C_MASTER_RESET, GPIO_MODE_OUTPUT);
    pal_os_event_init();
    err_code = optiga_util_open_application(&optiga_comms);
    ESP_ERROR_CHECK(err_code);

    err_code = optiga_util_read_data(CONFIG_OPTIGA_TRUST_X_CERT_SLOT, 0x09, my_cert, &my_cert_len);
    ESP_ERROR_CHECK(err_code);

    uint8_t curlim = 0x0e;
    err_code = optiga_util_write_data(eCURRENT_LIMITATION, OPTIGA_UTIL_WRITE_ONLY, 0, &curlim, 1);
    ESP_ERROR_CHECK(err_code);
}

static void wifi_conn_cb(void)
{
    xTaskCreate(&connect_server_task, "connect_server_task", 8192, NULL, 5, NULL);
}

static void wifi_failed_cb(void)
{
    ESP_LOGE(TAG, "wifi failed");
}

void app_main(void)
{
    init_hw();

    connect_wifi_params_t p = {
        .on_connected = wifi_conn_cb,
        .on_failed = wifi_failed_cb};
    connect_wifi(p);
}