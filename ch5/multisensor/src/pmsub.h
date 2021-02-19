
#ifndef pmsub_h_
#define pmsub_h_

typedef void (*before_sleep_f)(void);
typedef void (*after_wakeup_f)(void);


typedef struct
{
    int pir_pin;

    before_sleep_f before_sleep;
    after_wakeup_f after_wakeup;
} pmsub_config_t;

extern "C"
{
    void pmsub_init(pmsub_config_t);
    void pmsub_update(bool from_isr);
    void pmsub_start(void);
}

#endif
