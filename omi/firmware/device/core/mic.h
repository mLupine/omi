#ifndef MIC_H
#define MIC_H

#include <stdbool.h>
#include <stdint.h>

typedef void (*mix_handler)(int16_t *);

/**
 * @brief Initialize the Microphone with specified gain level
 *
 * Initializes the microphone with HAL init, DMIC configuration, and gain setting
 *
 * @param gain_level Microphone gain level (0-8 for nRF5340, 0-255 for nRF52840)
 * @return 0 if successful, negative errno code if error
 */
int mic_start_with_gain(uint8_t gain_level);
void set_mic_callback(mix_handler _callback);

void mic_off();
void mic_on();
void mic_set_gain(uint8_t gain_level);
#endif
