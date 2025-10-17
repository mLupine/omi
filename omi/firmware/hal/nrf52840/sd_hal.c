#include "hal/include/sd_hal.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

LOG_MODULE_REGISTER(sd_hal, CONFIG_LOG_DEFAULT_LEVEL);

struct gpio_dt_spec sd_en_gpio_pin = {.port = DEVICE_DT_GET(DT_NODELABEL(gpio0)),
                                      .pin = 19,
                                      .dt_flags = GPIO_INT_DISABLE};

static bool sd_enabled = false;

int sd_hal_power_on(void)
{
    if (!gpio_is_ready_dt(&sd_en_gpio_pin)) {
        LOG_ERR("SD Enable Pin not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&sd_en_gpio_pin, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure SD Pin: %d", ret);
        return ret;
    }

    gpio_pin_set_dt(&sd_en_gpio_pin, 1);

    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio1)), 15, GPIO_OUTPUT);
    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio1)), 14, GPIO_INPUT);
    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio1)), 13, GPIO_OUTPUT);
    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio0)), 2, GPIO_OUTPUT_HIGH);

    const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi2));
    if (device_is_ready(spi_dev)) {
        pm_device_action_run(spi_dev, PM_DEVICE_ACTION_RESUME);
    }

    sd_enabled = true;
    LOG_INF("SD card powered on");
    return 0;
}

int sd_hal_power_off(void)
{
    const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi2));
    if (device_is_ready(spi_dev)) {
        pm_device_action_run(spi_dev, PM_DEVICE_ACTION_SUSPEND);
    }

    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio1)), 15, GPIO_DISCONNECTED);
    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio1)), 14, GPIO_DISCONNECTED);
    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio1)), 13, GPIO_DISCONNECTED);
    gpio_pin_configure(DEVICE_DT_GET(DT_NODELABEL(gpio0)), 2, GPIO_DISCONNECTED);

    gpio_pin_set_dt(&sd_en_gpio_pin, 0);

    sd_enabled = false;
    LOG_INF("SD card powered off");
    return 0;
}

bool sd_hal_is_enabled(void)
{
    return sd_enabled;
}

const struct device *sd_hal_get_device(void)
{
    return NULL;
}
