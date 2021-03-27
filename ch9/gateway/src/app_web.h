
#ifndef app_web_h_
#define app_web_h_

#include <stdbool.h>

typedef void (*set_switch_f)(bool);

void appweb_init(set_switch_f);
void appweb_start_server(void);


#endif