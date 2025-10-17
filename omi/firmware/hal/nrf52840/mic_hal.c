#include "hal/include/mic_hal.h"

#include <haly/nrfy_gpio.h>
#include <nrfx_pdm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mic_hal, CONFIG_LOG_DEFAULT_LEVEL);

#define PDM_PWR_PIN NRF_GPIO_PIN_MAP(1, 10)

int mic_hal_init(void)
{
    nrfy_gpio_cfg_output(PDM_PWR_PIN);
    return 0;
}

int mic_hal_start(void)
{
    nrfy_gpio_pin_set(PDM_PWR_PIN);
    return 0;
}

void mic_hal_stop(void)
{
    nrfy_gpio_pin_clear(PDM_PWR_PIN);
}

int mic_hal_set_gain(uint8_t gain_level)
{
    uint8_t hw_gain = gain_level;

    LOG_INF("Setting mic gain to 0x%02x", hw_gain);
    nrf_pdm_gain_set(NRF_PDM0, hw_gain, hw_gain);

    return 0;
}

void mic_hal_power_on(void)
{
    nrfy_gpio_pin_set(PDM_PWR_PIN);
}

void mic_hal_power_off(void)
{
    nrfy_gpio_pin_clear(PDM_PWR_PIN);
}
