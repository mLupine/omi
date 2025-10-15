/**
 * @file hal_audio_nrf5340.c
 * @brief HAL Audio Implementation for nRF5340 (Production Hardware)
 * 
 * This implementation wraps around existing production mic.c functionality
 * to provide HAL interface without modifying production code.
 */

#include "../../hal/hal_audio.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Include existing production headers - disabled for initial builds */
/* TODO: Re-enable when production integration is complete */
/*
#ifdef CONFIG_OMI_UNIFIED_BUILD
#include "../../../omi/src/lib/core/mic.h"
#include "../../../omi/src/lib/core/config.h"
#else
*/
/* Use minimal interface for now */
#define MIC_BUFFER_SAMPLES 1600  /* From production config */
typedef void (*mix_handler)(int16_t *buffer);

/* Placeholder function declarations - TODO: integrate with production code */
static int mic_init(mix_handler handler) { return 0; }
static int mic_start(void) { return 0; }
static int mic_stop(void) { return 0; }
static bool mic_is_running(void) { return false; }
/*
#endif
*/

LOG_MODULE_REGISTER(hal_audio_nrf5340, CONFIG_LOG_DEFAULT_LEVEL);

/* HAL state management */
static hal_audio_config_t current_config = {0};
static bool audio_initialized = false;
static bool audio_recording = false;

/* Bridge callback from production code to HAL callback */
static void production_mic_callback(int16_t *buffer)
{
    if (current_config.callback) {
        /* Production uses MIC_BUFFER_SAMPLES, convert to frames */
        size_t frames = MIC_BUFFER_SAMPLES;
        current_config.callback(buffer, frames, current_config.user_data);
    }
}

/* Production audio interface implementation */
static int nrf5340_audio_init(const hal_audio_config_t *config)
{
    if (!config) {
        return -EINVAL;
    }
    
    LOG_INF("Initializing nRF5340 audio with sample rate: %d Hz", config->sample_rate);
    
    /* Store configuration */
    current_config = *config;
    
    /* Initialize production microphone using existing code */
    int ret = mic_init(production_mic_callback);
    if (ret != 0) {
        LOG_ERR("Failed to initialize production microphone: %d", ret);
        return ret;
    }
    
    audio_initialized = true;
    return 0;
}

static int nrf5340_audio_start_recording(void)
{
    if (!audio_initialized) {
        LOG_ERR("Audio not initialized");
        return -ENODEV;
    }
    
    LOG_INF("Starting nRF5340 audio recording");
    
    /* Use existing production mic_start function */
    int ret = mic_start();
    if (ret == 0) {
        audio_recording = true;
    }
    
    return ret;
}

static int nrf5340_audio_stop_recording(void)
{
    if (!audio_recording) {
        return 0; /* Already stopped */
    }
    
    LOG_INF("Stopping nRF5340 audio recording");
    
    /* Use existing production mic_stop function */
    int ret = mic_stop();
    if (ret == 0) {
        audio_recording = false;
    }
    
    return ret;
}

static bool nrf5340_audio_is_recording(void)
{
    #ifdef CONFIG_OMI_UNIFIED_BUILD
    /* Use production function if available */
    return mic_is_running();
    #else
    /* Fallback to local state */
    return audio_recording;
    #endif
}

static int nrf5340_audio_set_gain(float gain_db)
{
    /* Production hardware may have gain control - implement if available */
    LOG_DBG("Setting audio gain to %.2f dB (nRF5340)", gain_db);
    
    /* TODO: Implement gain control if production hardware supports it */
    /* For now, return success as many devices don't have software gain */
    return 0;
}

static float nrf5340_audio_get_gain(void)
{
    /* Return default gain for production hardware */
    return 0.0f; /* 0 dB (unity gain) */
}

static int nrf5340_audio_get_stats(uint32_t *frames_processed, uint32_t *overruns, uint32_t *underruns)
{
    /* Production audio statistics - would need to be implemented in production code */
    if (frames_processed) *frames_processed = 0;
    if (overruns) *overruns = 0;
    if (underruns) *underruns = 0;
    
    /* TODO: Integrate with production monitoring system for real stats */
    return 0;
}

/* nRF5340 audio interface definition */
static const hal_audio_interface_t nrf5340_audio_interface = {
    .init = nrf5340_audio_init,
    .start_recording = nrf5340_audio_start_recording,
    .stop_recording = nrf5340_audio_stop_recording,
    .is_recording = nrf5340_audio_is_recording,
    .set_gain = nrf5340_audio_set_gain,
    .get_gain = nrf5340_audio_get_gain,
    .get_stats = nrf5340_audio_get_stats,
};

/* HAL interface getter */
const hal_audio_interface_t* hal_audio_get_interface(void)
{
    return &nrf5340_audio_interface;
}

/* Global HAL wrapper functions */
int hal_audio_init(void)
{
    LOG_INF("HAL Audio initialized for nRF5340");
    return 0;
}

int hal_audio_configure(const hal_audio_config_t *config)
{
    return nrf5340_audio_interface.init(config);
}

int hal_audio_start_recording(void)
{
    return nrf5340_audio_interface.start_recording();
}

int hal_audio_stop_recording(void)
{
    return nrf5340_audio_interface.stop_recording();
}

bool hal_audio_is_recording(void)
{
    return nrf5340_audio_interface.is_recording();
}

int hal_audio_set_gain(float gain_db)
{
    return nrf5340_audio_interface.set_gain(gain_db);
}

float hal_audio_get_gain(void)
{
    return nrf5340_audio_interface.get_gain();
}

int hal_audio_get_stats(uint32_t *frames_processed, uint32_t *overruns, uint32_t *underruns)
{
    return nrf5340_audio_interface.get_stats(frames_processed, overruns, underruns);
}