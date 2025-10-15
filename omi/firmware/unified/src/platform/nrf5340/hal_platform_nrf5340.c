/**
 * @file hal_platform_nrf5340.c
 * @brief HAL Platform Implementation for nRF5340 (Production Hardware)
 * 
 * This implementation provides HAL services for the production Omi hardware
 * while preserving existing production code functionality.
 */

#include "../../hal/hal_platform.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(hal_platform_nrf5340, CONFIG_LOG_DEFAULT_LEVEL);

/* Production platform capabilities */
static const hal_platform_info_t nrf5340_platform_info = {
    .platform = HAL_PLATFORM_OMI_NRF5340,
    .cpu_frequency_hz = 128000000,  /* 128 MHz */
    .ram_size_bytes = 512 * 1024,   /* 512 KB RAM */
    .flash_size_bytes = 1024 * 1024, /* 1 MB Flash */
    .dual_core_support = true,
    .wifi_available = true,         /* WiFi capability via nRF70 */
    .external_flash_available = true,
    .usb_support = false,           /* No USB on production hardware */
    .max_audio_buffer_size = 4096,  /* Larger buffer for dual-core */
    .max_ble_mtu = 498              /* Production BLE MTU */
};

int hal_platform_init(void)
{
    LOG_INF("Initializing HAL for nRF5340 production platform");
    
    /* Production platform is already initialized by existing code */
    /* This HAL just provides unified interface without changing existing behavior */
    
    return 0;
}

hal_platform_type_t hal_platform_get_type(void)
{
    return HAL_PLATFORM_OMI_NRF5340;
}

const hal_platform_info_t* hal_platform_get_info(void)
{
    return &nrf5340_platform_info;
}

bool hal_platform_feature_available(const char* feature_name)
{
    if (!feature_name) {
        return false;
    }
    
    /* Check common features available on production hardware */
    if (strcmp(feature_name, "wifi") == 0) {
        return true;
    } else if (strcmp(feature_name, "dual_core") == 0) {
        return true;
    } else if (strcmp(feature_name, "external_flash") == 0) {
        return true;
    } else if (strcmp(feature_name, "advanced_audio") == 0) {
        return true;
    } else if (strcmp(feature_name, "hardware_crypto") == 0) {
        return true;
    } else if (strcmp(feature_name, "usb") == 0) {
        return false;  /* No USB on production */
    } else if (strcmp(feature_name, "haptic") == 0) {
        return true;   /* Production has haptic feedback */
    }
    
    LOG_WRN("Unknown feature requested: %s", feature_name);
    return false;
}

int hal_platform_early_init(void)
{
    /* Early initialization hook for production platform */
    /* Existing production initialization code runs before this */
    
    LOG_DBG("nRF5340 platform early initialization");
    
    /* Any additional platform-specific setup can go here */
    /* For now, rely on existing production initialization */
    
    return 0;
}