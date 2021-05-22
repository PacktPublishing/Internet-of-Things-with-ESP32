
#ifndef app_aws_h_
#define app_aws_h_

#include <stdint.h>

#define AWS_THING_NAME "myhome_fan1"

typedef void (*fan_state_changed_f)(uint8_t);

void appaws_init(fan_state_changed_f);
void appaws_connect(void *);
void appaws_publish(uint8_t);


#endif