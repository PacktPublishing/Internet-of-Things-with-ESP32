#ifndef app_temp_h_
#define app_temp_h_

#define DHT11_PIN 17

// temperature & humidity
typedef void (*temp_ready_f)(int, int);

void apptemp_init(temp_ready_f cb);

#endif