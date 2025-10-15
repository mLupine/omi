/**
 * @file main_unified.c
 * @brief Unified Main Entry Point for DevKit v2
 * 
 * This file provides a unified main entry point for DevKit v2 that uses the HAL
 * while preserving existing DevKit functionality. Production code continues to
 * use its original main.c without changes.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#ifdef CONFIG_OMI_PLATFORM_NRF52840

/* Include HAL interfaces */
#include "../hal/hal_platform.h"
#include "../hal/hal_audio.h" 
#include "../hal/hal_bluetooth.h"

/* Include existing DevKit headers (preserve existing functionality) */
#include "config.h"
#include "led.h"
#include "codec.h"
#include "transport.h"
#include "storage.h"
#include "button.h"

#ifdef CONFIG_OMI_ENABLE_USB
#include "usb.h"
#endif

#ifdef CONFIG_OMI_ENABLE_SPEAKER
#include "speaker.h"
#endif

LOG_MODULE_REGISTER(main_unified, CONFIG_LOG_DEFAULT_LEVEL);

/* Global state variables (preserve existing DevKit interface) */
bool is_connected = false;
bool is_charging = false;
bool is_off = false;

/* Audio callback bridge - connects HAL to existing DevKit codec */
static void audio_callback_bridge(int16_t *buffer, size_t frames, void *user_data)
{
    /* Use existing DevKit codec processing */
    int err = codec_receive_pcm(buffer, frames);
    if (err) {
        LOG_ERR("Failed to process PCM data: %d", err);
    }
}

/* Bluetooth callback bridge - connects HAL to existing DevKit transport */
static void bt_connection_callback(bool connected, void *user_data)
{
    is_connected = connected;
    LOG_INF("Bluetooth %s", connected ? "connected" : "disconnected");
}

static void bt_data_callback(hal_bt_data_type_t data_type, const uint8_t *data, 
                           size_t length, void *user_data)
{
    LOG_DBG("Received %zu bytes of data type %d", length, data_type);
    /* Handle incoming data as needed */
}

/* Boot LED sequence (preserve existing DevKit behavior) */
static void boot_led_sequence(void)
{
    /* Use existing DevKit LED functions */
    set_led_red(true);
    k_sleep(K_MSEC(600));
    set_led_red(false);
    k_sleep(K_MSEC(200));
    
    set_led_green(true);
    k_sleep(K_MSEC(600));
    set_led_green(false);
    k_sleep(K_MSEC(200));
    
    set_led_blue(true);
    k_sleep(K_MSEC(600));
    set_led_blue(false);
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
    LOG_INF("Platform: nRF52840 DevKit v2");
    LOG_INF("RAM: %u KB, Flash: %u KB", 
            platform_info->ram_size_bytes / 1024,
            platform_info->flash_size_bytes / 1024);
    LOG_INF("Max audio buffer: %u bytes", platform_info->max_audio_buffer_size);
    
    /* Initialize audio HAL with DevKit-appropriate settings */
    hal_audio_config_t audio_config = {
        .sample_rate = HAL_AUDIO_SAMPLE_RATE_16KHZ,
        .format = HAL_AUDIO_FORMAT_PCM_S16LE,
        .channels = 1,  /* Mono for DevKit */
        .buffer_frames = platform_info->max_audio_buffer_size / sizeof(int16_t),
        .callback = audio_callback_bridge,
        .user_data = NULL
    };
    
    ret = hal_audio_configure(&audio_config);
    if (ret != 0) {
        LOG_ERR("Failed to configure audio HAL: %d", ret);
        return ret;
    }
    
    /* Initialize Bluetooth HAL */
    hal_bt_config_t bt_config = {
        .device_name = "Omi DevKit 2 Unified",
        .appearance = 22,
        .max_mtu = 498,
        .conn_callback = bt_connection_callback,
        .data_callback = bt_data_callback,
        .user_data = NULL
    };
    
    ret = hal_bluetooth_configure(&bt_config);
    if (ret != 0) {
        LOG_ERR("Failed to configure Bluetooth HAL: %d", ret);
        return ret;
    }
    
    return 0;
}

/* Initialize existing DevKit subsystems (preserve existing code) */
static int initialize_devkit_subsystems(void)
{
    int ret;
    
    /* Initialize existing DevKit modules in original order */
    
    /* Initialize codec (existing DevKit function) */
    ret = codec_init();
    if (ret != 0) {
        LOG_ERR("Failed to initialize codec: %d", ret);
        return ret;
    }
    
    /* Initialize transport (existing DevKit function) */
    ret = transport_init();
    if (ret != 0) {
        LOG_ERR("Failed to initialize transport: %d", ret);
        return ret;
    }
    
    /* Initialize storage (existing DevKit function) */
    ret = storage_init();
    if (ret != 0) {
        LOG_ERR("Failed to initialize storage: %d", ret);
        return ret;
    }
    
    /* Initialize button handling (existing DevKit function) */
    button_init();
    
    #ifdef CONFIG_OMI_ENABLE_USB
    /* Initialize USB (existing DevKit function) */
    usb_init();
    #endif
    
    #ifdef CONFIG_OMI_ENABLE_SPEAKER
    /* Initialize speaker (existing DevKit function) */
    speaker_init();
    #endif
    
    return 0;
}

/* Main entry point for unified DevKit v2 firmware */
int main(void)
{
    int ret;
    
    LOG_INF("Starting Omi DevKit v2 Unified Firmware");
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
    
    /* Initialize existing DevKit subsystems */
    ret = initialize_devkit_subsystems();
    if (ret != 0) {
        LOG_ERR("DevKit subsystems initialization failed: %d", ret);
        return ret;
    }
    
    /* Start Bluetooth advertising */
    ret = hal_bluetooth_start_advertising();
    if (ret != 0) {
        LOG_ERR("Failed to start advertising: %d", ret);
        return ret;
    }
    
    LOG_INF("DevKit v2 Unified initialization complete");
    LOG_INF("Waiting for connections...");
    
    /* Main application loop */
    while (1) {
        /* Let existing DevKit code handle the main loop */
        /* HAL provides unified interface on top */
        
        /* Check connection status periodically */
        if (hal_bluetooth_get_state() == HAL_BT_STATE_CONNECTED && !is_connected) {
            LOG_INF("Device connected via Bluetooth");
            is_connected = true;
            
            /* Start audio recording when connected */
            ret = hal_audio_start_recording();
            if (ret != 0) {
                LOG_ERR("Failed to start audio recording: %d", ret);
            }
        } else if (hal_bluetooth_get_state() != HAL_BT_STATE_CONNECTED && is_connected) {
            LOG_INF("Device disconnected");
            is_connected = false;
            
            /* Stop audio recording when disconnected */
            hal_audio_stop_recording();
        }
        
        /* Sleep to prevent busy waiting */
        k_sleep(K_MSEC(1000));
    }
    
    return 0;
}

#endif /* CONFIG_OMI_PLATFORM_NRF52840 */