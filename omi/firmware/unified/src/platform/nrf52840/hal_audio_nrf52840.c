/**
 * @file hal_audio_nrf52840.c
 * @brief HAL Audio Implementation for nRF52840 (DevKit v2 Hardware)
 * 
 * This implementation adapts existing DevKit v2 mic.c functionality
 * to work with the unified HAL interface.
 */

#include "../../hal/hal_audio.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Include existing DevKit headers - disabled for initial builds */
/* TODO: Re-enable when DevKit integration is complete */
/*
#ifdef CONFIG_OMI_UNIFIED_BUILD
#include "../../../devkit/src/mic.h"
#include "../../../devkit/src/config.h"
#else
*/
/* Use minimal interface for now */
#define MIC_BUFFER_SAMPLES 1600
typedef void (*mix_handler)(int16_t *buffer);

/* Placeholder function declarations - TODO: integrate with DevKit code */
static int mic_init(mix_handler handler) { return 0; }
static int mic_start(void) { return 0; }
static int mic_stop(void) { return 0; }
/*
#endif
*/

LOG_MODULE_REGISTER(hal_audio_nrf52840, CONFIG_LOG_DEFAULT_LEVEL);

/* HAL state management */
static hal_audio_config_t current_config = {0};
static bool audio_initialized = false;
static bool audio_recording = false;

/* Bridge callback from DevKit code to HAL callback */
static void devkit_mic_callback(int16_t *buffer)
{
    if (current_config.callback) {
        /* DevKit uses MIC_BUFFER_SAMPLES, convert to frames */
        size_t frames = MIC_BUFFER_SAMPLES;
        current_config.callback(buffer, frames, current_config.user_data);
    }
}

/* nRF52840 audio interface implementation */
static int nrf52840_audio_init(const hal_audio_config_t *config)
{
    if (!config) {
        return -EINVAL;
    }
    
    LOG_INF("Initializing nRF52840 audio with sample rate: %d Hz", config->sample_rate);
    
    /* Store configuration */
    current_config = *config;
    
    /* Initialize DevKit microphone using existing code */
    int ret = mic_init(devkit_mic_callback);
    if (ret != 0) {
        LOG_ERR("Failed to initialize DevKit microphone: %d", ret);
        return ret;
    }
    
    audio_initialized = true;
    return 0;
}

static int nrf52840_audio_start_recording(void)
{
    if (!audio_initialized) {
        LOG_ERR("Audio not initialized");
        return -ENODEV;
    }
    
    LOG_INF("Starting nRF52840 audio recording");
    
    /* Use existing DevKit mic_start function */
    int ret = mic_start();
    if (ret == 0) {
        audio_recording = true;
    }
    
    return ret;
}

static int nrf52840_audio_stop_recording(void)
{
    if (!audio_recording) {
        return 0; /* Already stopped */
    }
    
    LOG_INF("Stopping nRF52840 audio recording");
    
    /* Use existing DevKit mic_stop function */
    int ret = mic_stop();
    if (ret == 0) {
        audio_recording = false;
    }
    
    return ret;
}

static bool nrf52840_audio_is_recording(void)
{
    return audio_recording;
}

static int nrf52840_audio_set_gain(float gain_db)
{
    LOG_DBG("Setting audio gain to %.2f dB (nRF52840)", gain_db);
    
    /* DevKit hardware doesn't have software gain control */
    /* Return success for compatibility */
    return 0;
}

static float nrf52840_audio_get_gain(void)
{
    /* Return default gain for DevKit hardware */
    return 0.0f; /* 0 dB (unity gain) */
}

static int nrf52840_audio_get_stats(uint32_t *frames_processed, uint32_t *overruns, uint32_t *underruns)
{
    /* DevKit audio statistics - basic implementation */
    if (frames_processed) *frames_processed = 0;
    if (overruns) *overruns = 0;
    if (underruns) *underruns = 0;
    
    /* TODO: Integrate with DevKit monitoring if available */
    return 0;
}

/* nRF52840 audio interface definition */
static const hal_audio_interface_t nrf52840_audio_interface = {
    .init = nrf52840_audio_init,
    .start_recording = nrf52840_audio_start_recording,
    .stop_recording = nrf52840_audio_stop_recording,
    .is_recording = nrf52840_audio_is_recording,
    .set_gain = nrf52840_audio_set_gain,
    .get_gain = nrf52840_audio_get_gain,
    .get_stats = nrf52840_audio_get_stats,
};

/* HAL interface getter */
const hal_audio_interface_t* hal_audio_get_interface(void)
{
    return &nrf52840_audio_interface;
}

/* Global HAL wrapper functions */
int hal_audio_init(void)
{
    LOG_INF("HAL Audio initialized for nRF52840");
    return 0;
}

int hal_audio_configure(const hal_audio_config_t *config)
{
    return nrf52840_audio_interface.init(config);
}

int hal_audio_start_recording(void)
{
    return nrf52840_audio_interface.start_recording();
}

int hal_audio_stop_recording(void)
{
    return nrf52840_audio_interface.stop_recording();
}

bool hal_audio_is_recording(void)
{
    return nrf52840_audio_interface.is_recording();
}

int hal_audio_set_gain(float gain_db)
{
    return nrf52840_audio_interface.set_gain(gain_db);
}

float hal_audio_get_gain(void)
{
    return nrf52840_audio_interface.get_gain();
}

int hal_audio_get_stats(uint32_t *frames_processed, uint32_t *overruns, uint32_t *underruns)
{
    return nrf52840_audio_interface.get_stats(frames_processed, overruns, underruns);
}