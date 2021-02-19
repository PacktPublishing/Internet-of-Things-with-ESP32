
#include "uisub.h"
#include "pmsub.h"
#include "sesub.h"

#define SENSOR_BUS_SDA 21
#define SENSOR_BUS_SCL 22
#define OLED_SDA 32
#define OLED_SCL 33
#define BUZZER_PIN 17
#define PIR_MOTION_PIN 4
#define ROTENC_CLK_PIN 19
#define ROTENC_DT_PIN 18

static void update_power_man(void)
{
    pmsub_update(false);
}

static void alarm(void)
{
    uisub_beep(3);
}

static void init_subsystems(void)
{
    uisub_config_t ui_cfg = {
        .buzzer_pin = BUZZER_PIN,
        .rotenc_clk_pin = ROTENC_CLK_PIN,
        .rotenc_dt_pin = ROTENC_DT_PIN,
        .rotenc_changed = update_power_man,
        .oled_sda = OLED_SDA,
        .oled_scl = OLED_SCL,
    };
    uisub_init(ui_cfg);

    sesub_config_t se_cfg = {
        .sensor_sda = SENSOR_BUS_SDA,
        .sensor_scl = SENSOR_BUS_SCL,
        .temp_high = 30,
        .temp_low = 10,
        .new_sensor_reading = uisub_show,
        .temp_alarm = alarm,
    };
    sesub_init(se_cfg);

    pmsub_config_t pm_cfg = {
        .pir_pin = PIR_MOTION_PIN,
        .before_sleep = uisub_sleep,
        .after_wakeup = uisub_resume,
    };
    pmsub_init(pm_cfg);
}

extern "C" void app_main(void)
{
    init_subsystems();

    uisub_beep(2);

    sesub_start();
    pmsub_start();
}