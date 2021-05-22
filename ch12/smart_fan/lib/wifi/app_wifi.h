#ifndef app_wifi_h_
#define app_wifi_h_

typedef void (*on_connected_f)(void);
typedef void (*on_failed_f)(void);

typedef struct {
    on_connected_f on_connected;
    on_failed_f on_failed;
} connect_wifi_params_t;

void appwifi_connect(connect_wifi_params_t);

#endif