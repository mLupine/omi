/**
 * @file hal_platform.h
 * @brief Hardware Abstraction Layer - Platform Detection and Information
 * 
 * This HAL is designed to wrap around existing production code with minimal changes.
 * Production code continues to work as-is, while DevKit code is adapted to use the same interfaces.
 */

#ifndef HAL_PLATFORM_H
#define HAL_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Platform type enumeration
 */
typedef enum {
    HAL_PLATFORM_OMI_NRF5340,      /**< Production Omi hardware (nRF5340) */
    HAL_PLATFORM_XIAO_NRF52840,    /**< DevKit v2 hardware (nRF52840) */
    HAL_PLATFORM_UNKNOWN
} hal_platform_type_t;

/**
 * @brief Platform capabilities structure
 */
typedef struct {
    hal_platform_type_t platform;
    uint32_t cpu_frequency_hz;
    uint32_t ram_size_bytes;
    uint32_t flash_size_bytes;
    bool dual_core_support;
    bool wifi_available;
    bool external_flash_available;
    bool usb_support;
    uint32_t max_audio_buffer_size;
    uint32_t max_ble_mtu;
} hal_platform_info_t;

/**
 * @brief Initialize the HAL platform layer
 * @return 0 on success, negative on error
 */
int hal_platform_init(void);

/**
 * @brief Get current platform type
 * @return Platform type
 */
hal_platform_type_t hal_platform_get_type(void);

/**
 * @brief Get platform information
 * @return Pointer to platform info structure
 */
const hal_platform_info_t* hal_platform_get_info(void);

/**
 * @brief Check if a feature is available on current platform
 * @param feature_name Feature name string
 * @return true if available, false otherwise
 */
bool hal_platform_feature_available(const char* feature_name);

/**
 * @brief Platform-specific initialization hook
 * Called during system startup, can be used for platform-specific setup
 * @return 0 on success, negative on error
 */
int hal_platform_early_init(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_PLATFORM_H */