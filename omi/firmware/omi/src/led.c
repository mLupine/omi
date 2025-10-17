#include "lib/core/led.h"

#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

#include "lib/core/settings.h"
#include "lib/core/utils.h"

LOG_MODULE_REGISTER(led, CONFIG_LOG_DEFAULT_LEVEL);

int led_start()
{
    return led_hal_init();
}

void set_led_red(bool on)
{
    if (on) {
        uint8_t ratio = app_settings_get_dim_ratio();
        if (ratio > 100) {
            ratio = 100;
        }
        led_hal_set_pwm(LED_RED, ratio);
    } else {
        led_hal_set(LED_RED, false);
    }
}

void set_led_green(bool on)
{
    if (on) {
        uint8_t ratio = app_settings_get_dim_ratio();
        if (ratio > 100) {
            ratio = 100;
        }
        led_hal_set_pwm(LED_GREEN, ratio);
    } else {
        led_hal_set(LED_GREEN, false);
    }
}

void set_led_blue(bool on)
{
    if (on) {
        uint8_t ratio = app_settings_get_dim_ratio();
        if (ratio > 100) {
            ratio = 100;
        }
        led_hal_set_pwm(LED_BLUE, ratio);
    } else {
        led_hal_set(LED_BLUE, false);
    }
}

void set_led_pwm(led_color_t color, uint8_t level)
{
    led_hal_set_pwm(color, level);
}

void led_off(void)
{
    set_led_red(false);
    k_msleep(10);
    set_led_green(false);
    k_msleep(10);
    set_led_blue(false);
}
