#ifndef MIC_HAL_H
#define MIC_HAL_H

#include <stdint.h>

int mic_hal_init(void);

int mic_hal_start(void);

void mic_hal_stop(void);

int mic_hal_set_gain(uint8_t gain_level);

void mic_hal_power_on(void);

void mic_hal_power_off(void);

#endif
