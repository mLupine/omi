#ifndef LED_HAL_H
#define LED_HAL_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    LED_RED,
    LED_GREEN,
    LED_BLUE
} led_color_t;

int led_hal_init(void);

void led_hal_set(led_color_t color, bool on);

void led_hal_set_pwm(led_color_t color, uint8_t level);

#endif
