#include "hal/include/sd_hal.h"

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/device.h>

LOG_MODULE_REGISTER(sd_hal, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *const sd_dev = DEVICE_DT_GET(DT_NODELABEL(sdhc0));
static const struct gpio_dt_spec sd_en = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(sdcard_en_pin), gpios, {0});

static bool sd_enabled = false;

int sd_hal_power_on(void)
{
    int ret;

    ret = gpio_pin_configure_dt(&sd_en, GPIO_OUTPUT);
    if (ret < 0) {
        LOG_ERR("Failed to configure SD enable GPIO: %d", ret);
        return ret;
    }

    ret = gpio_pin_set_dt(&sd_en, 1);
    if (ret < 0) {
        LOG_ERR("Failed to set SD enable GPIO: %d", ret);
        return ret;
    }

    ret = pm_device_action_run(sd_dev, PM_DEVICE_ACTION_RESUME);
    if (ret < 0) {
        LOG_ERR("Failed to resume SD device: %d", ret);
        return ret;
    }

    sd_enabled = true;
    return 0;
}

int sd_hal_power_off(void)
{
    int ret;

    ret = pm_device_action_run(sd_dev, PM_DEVICE_ACTION_SUSPEND);
    if (ret < 0) {
        LOG_ERR("Failed to suspend SD device: %d", ret);
        return ret;
    }

    sd_enabled = false;
    return 0;
}

bool sd_hal_is_enabled(void)
{
    return sd_enabled;
}

const struct device *sd_hal_get_device(void)
{
    return sd_dev;
}
