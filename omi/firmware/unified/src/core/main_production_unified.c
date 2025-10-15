/**
 * @file main_production_unified.c
 * @brief Unified Main Entry Point for Production nRF5340
 * 
 * This file provides a unified main entry point for production hardware that uses the HAL
 * while demonstrating integration with existing production code patterns.
 * This is a minimal implementation to get builds working.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_OMI_PLATFORM_NRF5340

/* Include HAL interfaces */
#include "../hal/hal_platform.h"
#include "../hal/hal_audio.h" 
#include "../hal/hal_bluetooth.h"

LOG_MODULE_REGISTER(main_production_unified, CONFIG_LOG_DEFAULT_LEVEL);

/* Global state variables (minimal for now) */
bool is_connected = false;
bool is_charging = false;
bool is_off = false;

/* Boot LED sequence placeholder */
static void boot_led_sequence(void)
{
    LOG_INF("Boot LED sequence (placeholder)");
    k_sleep(K_MSEC(1000));
}

/* Initialize HAL subsystems */
static int initialize_hal(void)
{
    int ret;
    
    /* Initialize platform HAL */
    ret = hal_platform_init();
    if (ret != 0) {
        LOG_ERR("Failed to initialize platform HAL: %d", ret);
        return ret;
    }
    
    /* Log platform information */
    const hal_platform_info_t *platform_info = hal_platform_get_info();
    LOG_INF("Platform: nRF5340 Production");
    LOG_INF("RAM: %u KB, Flash: %u KB", 
            platform_info->ram_size_bytes / 1024,
            platform_info->flash_size_bytes / 1024);
    LOG_INF("Features: WiFi=%s, Dual-core=%s", 
            platform_info->wifi_available ? "yes" : "no",
            platform_info->dual_core_support ? "yes" : "no");
    
    return 0;
}

/* Main entry point for unified production firmware */
int main(void)
{
    int ret;
    
    LOG_INF("Starting Omi Production Unified Firmware");
    LOG_INF("Platform: nRF5340 (Production Hardware)");
    LOG_INF("SDK Version: 2.9.0 (Unified Build)");
    
    /* Boot LED sequence */
    boot_led_sequence();
    
    /* Early platform initialization */
    ret = hal_platform_early_init();
    if (ret != 0) {
        LOG_ERR("Platform early init failed: %d", ret);
        return ret;
    }
    
    /* Initialize HAL subsystems */
    ret = initialize_hal();
    if (ret != 0) {
        LOG_ERR("HAL initialization failed: %d", ret);
        return ret;
    }
    
    LOG_INF("Production Unified initialization complete");
    LOG_INF("HAL successfully initialized for nRF5340");
    
    /* Main application loop - minimal for now */
    while (1) {
        /* Basic heartbeat */
        LOG_DBG("Production firmware running...");
        
        /* Sleep to prevent busy waiting */
        k_sleep(K_MSEC(5000));
    }
    
    return 0;
}

#else
#error "This file should only be compiled for nRF5340 platform"
#endif /* CONFIG_OMI_PLATFORM_NRF5340 */