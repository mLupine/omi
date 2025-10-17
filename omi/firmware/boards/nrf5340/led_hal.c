#include "device/hal/led_hal.h"

#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_hal, CONFIG_LOG_DEFAULT_LEVEL);

static const struct pwm_dt_spec led_red = PWM_DT_SPEC_GET(DT_NODELABEL(led_red));
static const struct pwm_dt_spec led_green = PWM_DT_SPEC_GET(DT_NODELABEL(led_green));
static const struct pwm_dt_spec led_blue = PWM_DT_SPEC_GET(DT_NODELABEL(led_blue));

static const struct pwm_dt_spec *get_led_spec(led_color_t color)
{
    switch (color) {
    case LED_RED:
        return &led_red;
    case LED_GREEN:
        return &led_green;
    case LED_BLUE:
        return &led_blue;
    default:
        return NULL;
    }
}

int led_hal_init(void)
{
    if (!pwm_is_ready_dt(&led_red)) {
        LOG_ERR("Red LED PWM device not ready");
        return -ENODEV;
    }
    if (!pwm_is_ready_dt(&led_green)) {
        LOG_ERR("Green LED PWM device not ready");
        return -ENODEV;
    }
    if (!pwm_is_ready_dt(&led_blue)) {
        LOG_ERR("Blue LED PWM device not ready");
        return -ENODEV;
    }
    LOG_INF("LEDs (PWM) initialized");
    return 0;
}

void led_hal_set(led_color_t color, bool on)
{
    const struct pwm_dt_spec *led = get_led_spec(color);
    if (!led || !pwm_is_ready_dt(led)) {
        LOG_ERR("LED device not ready");
        return;
    }

    uint32_t pulse_width_ns = on ? led->period : 0;
    pwm_set_pulse_dt(led, pulse_width_ns);
}

void led_hal_set_pwm(led_color_t color, uint8_t level)
{
    const struct pwm_dt_spec *led = get_led_spec(color);
    if (!led || !pwm_is_ready_dt(led)) {
        LOG_ERR("LED device not ready");
        return;
    }

    if (level > 100) {
        level = 100;
    }

    uint32_t pulse_width_ns = (led->period * level) / 100;
    pwm_set_pulse_dt(led, pulse_width_ns);
}
