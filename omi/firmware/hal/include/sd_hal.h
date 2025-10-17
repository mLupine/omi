#ifndef SD_HAL_H
#define SD_HAL_H

#include <stdbool.h>

int sd_hal_power_on(void);

int sd_hal_power_off(void);

bool sd_hal_is_enabled(void);

const struct device *sd_hal_get_device(void);

#endif
