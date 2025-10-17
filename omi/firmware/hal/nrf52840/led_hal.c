#include "hal/include/led_hal.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_hal, CONFIG_LOG_DEFAULT_LEVEL);

static const struct gpio_dt_spec led_red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led_blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

static const struct gpio_dt_spec *get_led_spec(led_color_t color)
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
    int ret;

    if (!gpio_is_ready_dt(&led_red)) {
        LOG_ERR("Red LED GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&led_red, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure red LED: %d", ret);
        return ret;
    }

    if (!gpio_is_ready_dt(&led_green)) {
        LOG_ERR("Green LED GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&led_green, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure green LED: %d", ret);
        return ret;
    }

    if (!gpio_is_ready_dt(&led_blue)) {
        LOG_ERR("Blue LED GPIO device not ready");
        return -ENODEV;
    }
    ret = gpio_pin_configure_dt(&led_blue, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure blue LED: %d", ret);
        return ret;
    }

    LOG_INF("LEDs (GPIO) initialized");
    return 0;
}

void led_hal_set(led_color_t color, bool on)
{
    const struct gpio_dt_spec *led = get_led_spec(color);
    if (!led) {
        LOG_ERR("Invalid LED color");
        return;
    }

    gpio_pin_set_dt(led, on);
}

void led_hal_set_pwm(led_color_t color, uint8_t level)
{
    led_hal_set(color, level > 0);
}
