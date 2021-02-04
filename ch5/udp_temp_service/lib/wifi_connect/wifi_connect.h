#ifndef wifi_connect_h_
#define wifi_connect_h_

typedef void (*on_connected_f)(void);
typedef void (*on_failed_f)(void);

typedef struct {
    on_connected_f on_connected;
    on_failed_f on_failed;
} connect_wifi_params_t;

void connect_wifi(connect_wifi_params_t);

#endif