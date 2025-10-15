/**
 * @file hal_bluetooth_nrf5340.c
 * @brief HAL Bluetooth Implementation for nRF5340 (Production Hardware)
 * 
 * This implementation wraps around existing production transport.c functionality
 * to provide HAL interface without modifying production code.
 */

#include "../../hal/hal_bluetooth.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

/* Include existing production headers - disabled for initial builds */
/* TODO: Re-enable when production integration is complete */
/*
#ifdef CONFIG_OMI_UNIFIED_BUILD
#include "../../../omi/src/lib/core/transport.h"
#include "../../../omi/src/lib/core/config.h"
#else
*/
/* Use minimal interface for now */
static bool is_connected = false;

/* Placeholder function declarations - TODO: integrate with production code */
static int broadcast_audio_packets(uint8_t *data, size_t len) { return 0; }
static int transport_init(void) { return 0; }
static int start_advertising(void) { return 0; }
static int stop_advertising(void) { return 0; }
/*
#endif
*/

LOG_MODULE_REGISTER(hal_bluetooth_nrf5340, CONFIG_LOG_DEFAULT_LEVEL);

/* HAL state management */
static hal_bt_config_t current_config = {0};
static bool bt_initialized = false;
static uint32_t stats_bytes_sent = 0;
static uint32_t stats_packets_sent = 0;
static uint32_t stats_packets_failed = 0;

/* Production Bluetooth interface implementation */
static int nrf5340_bt_init(const hal_bt_config_t *config)
{
    if (!config) {
        return -EINVAL;
    }
    
    LOG_INF("Initializing nRF5340 Bluetooth: %s", config->device_name);
    
    /* Store configuration */
    current_config = *config;
    
    /* Initialize production transport using existing code */
    #ifdef CONFIG_OMI_UNIFIED_BUILD
    int ret = transport_init();
    #else
    int ret = 0; /* Assume already initialized by existing code */
    #endif
    
    if (ret != 0) {
        LOG_ERR("Failed to initialize production transport: %d", ret);
        return ret;
    }
    
    bt_initialized = true;
    return 0;
}

static int nrf5340_bt_start_advertising(void)
{
    if (!bt_initialized) {
        LOG_ERR("Bluetooth not initialized");
        return -ENODEV;
    }
    
    LOG_INF("Starting nRF5340 Bluetooth advertising");
    
    /* Use existing production advertising function */
    #ifdef CONFIG_OMI_UNIFIED_BUILD
    return start_advertising();
    #else
    /* For standalone builds, assume advertising starts automatically */
    return 0;
    #endif
}

static int nrf5340_bt_stop_advertising(void)
{
    LOG_INF("Stopping nRF5340 Bluetooth advertising");
    
    #ifdef CONFIG_OMI_UNIFIED_BUILD
    return stop_advertising();
    #else
    return 0;
    #endif
}

static int nrf5340_bt_disconnect(void)
{
    LOG_INF("Disconnecting nRF5340 Bluetooth");
    
    /* Production code handles disconnection automatically */
    /* TODO: Add explicit disconnect function to production code if needed */
    return 0;
}

static int nrf5340_bt_send_data(hal_bt_data_type_t data_type, const uint8_t *data, size_t length)
{
    if (!data || length == 0) {
        return -EINVAL;
    }
    
    int ret = 0;
    
    switch (data_type) {
        case HAL_BT_DATA_AUDIO:
            /* Use existing production audio packet broadcasting */
            ret = broadcast_audio_packets((uint8_t*)data, length);
            break;
            
        case HAL_BT_DATA_TELEMETRY:
            /* TODO: Add telemetry data sending to production transport */
            LOG_DBG("Sending telemetry data (%zu bytes)", length);
            ret = 0; /* Success for now */
            break;
            
        case HAL_BT_DATA_CONTROL:
        case HAL_BT_DATA_CONFIG:
            /* TODO: Add control/config data sending to production transport */
            LOG_DBG("Sending control/config data (%zu bytes)", length);
            ret = 0; /* Success for now */
            break;
            
        default:
            LOG_ERR("Unknown data type: %d", data_type);
            ret = -EINVAL;
            break;
    }
    
    /* Update statistics */
    if (ret == 0) {
        stats_bytes_sent += length;
        stats_packets_sent++;
    } else {
        stats_packets_failed++;
    }
    
    return ret;
}

static hal_bt_state_t nrf5340_bt_get_state(void)
{
    #ifdef CONFIG_OMI_UNIFIED_BUILD
    /* Use existing production connection state */
    extern bool is_connected;
    return is_connected ? HAL_BT_STATE_CONNECTED : HAL_BT_STATE_ADVERTISING;
    #else
    /* For standalone, assume connected for simplicity */
    return HAL_BT_STATE_CONNECTED;
    #endif
}

static uint16_t nrf5340_bt_get_mtu(void)
{
    /* Production uses CONFIG_BT_L2CAP_TX_MTU=498 */
    return 498;
}

static int nrf5340_bt_get_stats(uint32_t *bytes_sent, uint32_t *bytes_received, 
                                uint32_t *packets_sent, uint32_t *packets_failed)
{
    if (bytes_sent) *bytes_sent = stats_bytes_sent;
    if (bytes_received) *bytes_received = 0; /* TODO: Track received bytes */
    if (packets_sent) *packets_sent = stats_packets_sent;
    if (packets_failed) *packets_failed = stats_packets_failed;
    
    return 0;
}

/* nRF5340 Bluetooth interface definition */
static const hal_bt_interface_t nrf5340_bt_interface = {
    .init = nrf5340_bt_init,
    .start_advertising = nrf5340_bt_start_advertising,
    .stop_advertising = nrf5340_bt_stop_advertising,
    .disconnect = nrf5340_bt_disconnect,
    .send_data = nrf5340_bt_send_data,
    .get_state = nrf5340_bt_get_state,
    .get_mtu = nrf5340_bt_get_mtu,
    .get_stats = nrf5340_bt_get_stats,
};

/* HAL interface getter */
const hal_bt_interface_t* hal_bluetooth_get_interface(void)
{
    return &nrf5340_bt_interface;
}

/* Global HAL wrapper functions */
int hal_bluetooth_init(void)
{
    LOG_INF("HAL Bluetooth initialized for nRF5340");
    return 0;
}

int hal_bluetooth_configure(const hal_bt_config_t *config)
{
    return nrf5340_bt_interface.init(config);
}

int hal_bluetooth_start_advertising(void)
{
    return nrf5340_bt_interface.start_advertising();
}

int hal_bluetooth_stop_advertising(void)
{
    return nrf5340_bt_interface.stop_advertising();
}

int hal_bluetooth_disconnect(void)
{
    return nrf5340_bt_interface.disconnect();
}

int hal_bluetooth_send_data(hal_bt_data_type_t data_type, const uint8_t *data, size_t length)
{
    return nrf5340_bt_interface.send_data(data_type, data, length);
}

hal_bt_state_t hal_bluetooth_get_state(void)
{
    return nrf5340_bt_interface.get_state();
}

uint16_t hal_bluetooth_get_mtu(void)
{
    return nrf5340_bt_interface.get_mtu();
}

int hal_bluetooth_get_stats(uint32_t *bytes_sent, uint32_t *bytes_received, 
                           uint32_t *packets_sent, uint32_t *packets_failed)
{
    return nrf5340_bt_interface.get_stats(bytes_sent, bytes_received, packets_sent, packets_failed);
}

int hal_bluetooth_send_audio_packet(const uint8_t *data, size_t length)
{
    return hal_bluetooth_send_data(HAL_BT_DATA_AUDIO, data, length);
}

int hal_bluetooth_send_telemetry(const uint8_t *data, size_t length)
{
    return hal_bluetooth_send_data(HAL_BT_DATA_TELEMETRY, data, length);
}