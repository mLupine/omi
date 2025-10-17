#include "hal/include/mic_hal.h"

#include <nrfx_pdm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mic_hal, CONFIG_LOG_DEFAULT_LEVEL);

int mic_hal_init(void)
{
    return 0;
}

int mic_hal_start(void)
{
    return 0;
}

void mic_hal_stop(void)
{
}

int mic_hal_set_gain(uint8_t gain_level)
{
    static const uint8_t gain_map[9] = {
        0x00,
        0x14,
        0x1E,
        0x28,
        0x2E,
        0x32,
        0x3C,
        0x46,
        0x50
    };

    if (gain_level > 8) {
        gain_level = 8;
    }

    uint8_t hw_gain = gain_map[gain_level];

    LOG_INF("Setting mic gain to level %u (0x%02x)", gain_level, hw_gain);

#ifdef NRF_PDM0_S
    nrf_pdm_gain_set(NRF_PDM0_S, hw_gain, hw_gain);
#else
    nrf_pdm_gain_set(NRF_PDM0_NS, hw_gain, hw_gain);
#endif

    return 0;
}

void mic_hal_power_on(void)
{
}

void mic_hal_power_off(void)
{
}
