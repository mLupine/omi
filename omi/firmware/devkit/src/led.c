#include "led.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "device/hal/led_hal.h"
#include "utils.h"

LOG_MODULE_REGISTER(led, CONFIG_LOG_DEFAULT_LEVEL);

int led_start()
{
    return led_hal_init();
}

void set_led_red(bool on)
{
    led_hal_set(LED_RED, on);
}

void set_led_green(bool on)
{
    led_hal_set(LED_GREEN, on);
}

void set_led_blue(bool on)
{
    led_hal_set(LED_BLUE, on);
}
