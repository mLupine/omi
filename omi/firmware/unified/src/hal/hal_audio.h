/**
 * @file hal_audio.h
 * @brief Hardware Abstraction Layer - Audio Interface
 * 
 * This interface wraps around existing production audio code to provide
 * a unified API while preserving production optimizations.
 */

#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Audio sample rate enumeration
 */
typedef enum {
    HAL_AUDIO_SAMPLE_RATE_8KHZ = 8000,
    HAL_AUDIO_SAMPLE_RATE_16KHZ = 16000,
    HAL_AUDIO_SAMPLE_RATE_22KHZ = 22050,
    HAL_AUDIO_SAMPLE_RATE_44KHZ = 44100
} hal_audio_sample_rate_t;

/**
 * @brief Audio format enumeration
 */
typedef enum {
    HAL_AUDIO_FORMAT_PCM_S16LE,    /**< 16-bit signed PCM, little endian */
    HAL_AUDIO_FORMAT_PCM_S24LE,    /**< 24-bit signed PCM, little endian */
    HAL_AUDIO_FORMAT_PCM_S32LE     /**< 32-bit signed PCM, little endian */
} hal_audio_format_t;

/**
 * @brief Audio callback function type
 * This matches the existing production callback signature to minimize changes
 * 
 * @param buffer Pointer to audio data buffer
 * @param frames Number of frames in buffer
 * @param user_data User-provided data pointer
 */
typedef void (*hal_audio_callback_t)(int16_t *buffer, size_t frames, void *user_data);

/**
 * @brief Audio configuration structure
 */
typedef struct {
    hal_audio_sample_rate_t sample_rate;
    hal_audio_format_t format;
    uint8_t channels;              /**< Number of audio channels (1=mono, 2=stereo) */
    size_t buffer_frames;          /**< Buffer size in frames */
    hal_audio_callback_t callback; /**< Audio data callback */
    void *user_data;               /**< User data for callback */
} hal_audio_config_t;

/**
 * @brief Audio interface structure
 * 
 * Production code uses these functions through the existing mic.h interface,
 * while DevKit code uses this HAL directly. The implementation bridges to
 * existing production functions to minimize code changes.
 */
typedef struct {
    /**
     * @brief Initialize audio subsystem
     * @param config Audio configuration
     * @return 0 on success, negative on error
     */
    int (*init)(const hal_audio_config_t *config);
    
    /**
     * @brief Start audio recording
     * @return 0 on success, negative on error
     */
    int (*start_recording)(void);
    
    /**
     * @brief Stop audio recording
     * @return 0 on success, negative on error
     */
    int (*stop_recording)(void);
    
    /**
     * @brief Check if audio is currently recording
     * @return true if recording, false otherwise
     */
    bool (*is_recording)(void);
    
    /**
     * @brief Set audio gain
     * @param gain_db Gain in decibels
     * @return 0 on success, negative on error
     */
    int (*set_gain)(float gain_db);
    
    /**
     * @brief Get current audio gain
     * @return Current gain in decibels
     */
    float (*get_gain)(void);
    
    /**
     * @brief Get audio statistics
     * @param frames_processed Output: number of frames processed
     * @param overruns Output: number of buffer overruns
     * @param underruns Output: number of buffer underruns
     * @return 0 on success, negative on error
     */
    int (*get_stats)(uint32_t *frames_processed, uint32_t *overruns, uint32_t *underruns);
    
} hal_audio_interface_t;

/**
 * @brief Get the platform-specific audio interface
 * @return Pointer to audio interface structure
 */
const hal_audio_interface_t* hal_audio_get_interface(void);

/**
 * @brief Initialize audio HAL
 * This function sets up the platform-specific audio interface
 * @return 0 on success, negative on error
 */
int hal_audio_init(void);

/* Convenience wrapper functions that delegate to the interface */
int hal_audio_configure(const hal_audio_config_t *config);
int hal_audio_start_recording(void);
int hal_audio_stop_recording(void);
bool hal_audio_is_recording(void);
int hal_audio_set_gain(float gain_db);
float hal_audio_get_gain(void);
int hal_audio_get_stats(uint32_t *frames_processed, uint32_t *overruns, uint32_t *underruns);

#ifdef __cplusplus
}
#endif

#endif /* HAL_AUDIO_H */