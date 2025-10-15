/**
 * @file hal_platform_nrf52840.c
 * @brief HAL Platform Implementation for nRF52840 (DevKit v2 Hardware)
 * 
 * This implementation adapts existing DevKit v2 code to work with the unified HAL
 * while upgrading to SDK v2.9.0 for compatibility with production code.
 */

#include "../../hal/hal_platform.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(hal_platform_nrf52840, CONFIG_LOG_DEFAULT_LEVEL);

/* DevKit v2 platform capabilities (more constrained than production) */
static const hal_platform_info_t nrf52840_platform_info = {
    .platform = HAL_PLATFORM_XIAO_NRF52840,
    .cpu_frequency_hz = 64000000,   /* 64 MHz */
    .ram_size_bytes = 256 * 1024,   /* 256 KB RAM (constrained) */
    .flash_size_bytes = 1024 * 1024, /* 1 MB Flash */
    .dual_core_support = false,     /* Single core */
    .wifi_available = false,        /* No WiFi on DevKit */
    .external_flash_available = false, /* Uses SD card instead */
    .usb_support = true,            /* USB available on DevKit */
    .max_audio_buffer_size = 2048,  /* Smaller buffer for single-core */
    .max_ble_mtu = 498              /* Same BLE MTU as production */
};

int hal_platform_init(void)
{
    LOG_INF("Initializing HAL for nRF52840 DevKit v2 platform");
    
    /* DevKit platform initialization - existing code continues to work */
    /* This HAL provides unified interface on top of existing DevKit code */
    
    return 0;
}

hal_platform_type_t hal_platform_get_type(void)
{
    return HAL_PLATFORM_XIAO_NRF52840;
}

const hal_platform_info_t* hal_platform_get_info(void)
{
    return &nrf52840_platform_info;
}

bool hal_platform_feature_available(const char* feature_name)
{
    if (!feature_name) {
        return false;
    }
    
    /* Check features available on DevKit v2 hardware */
    if (strcmp(feature_name, "wifi") == 0) {
        return false;  /* No WiFi on DevKit */
    } else if (strcmp(feature_name, "dual_core") == 0) {
        return false;  /* Single core only */
    } else if (strcmp(feature_name, "external_flash") == 0) {
        return false;  /* Uses SD card storage */
    } else if (strcmp(feature_name, "advanced_audio") == 0) {
        return false;  /* Limited audio processing on single core */
    } else if (strcmp(feature_name, "hardware_crypto") == 0) {
        return false;  /* Limited crypto hardware */
    } else if (strcmp(feature_name, "usb") == 0) {
        return true;   /* USB available on DevKit */
    } else if (strcmp(feature_name, "haptic") == 0) {
        return true;   /* DevKit v2 has haptic feedback */
    } else if (strcmp(feature_name, "sd_card") == 0) {
        return true;   /* DevKit uses SD card storage */
    } else if (strcmp(feature_name, "speaker") == 0) {
        return true;   /* DevKit has speaker support */
    }
    
    LOG_WRN("Unknown feature requested: %s", feature_name);
    return false;
}

int hal_platform_early_init(void)
{
    /* Early initialization hook for DevKit platform */
    LOG_DBG("nRF52840 DevKit v2 platform early initialization");
    
    /* DevKit-specific early setup */
    /* Most initialization is handled by existing DevKit code */
    
    return 0;
}