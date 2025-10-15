/**
 * @file hal_bluetooth.h
 * @brief Hardware Abstraction Layer - Bluetooth Interface
 * 
 * This interface wraps around existing production Bluetooth/transport code
 * to provide unified API while preserving production optimizations.
 */

#ifndef HAL_BLUETOOTH_H
#define HAL_BLUETOOTH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bluetooth connection state
 */
typedef enum {
    HAL_BT_STATE_DISCONNECTED,
    HAL_BT_STATE_ADVERTISING,
    HAL_BT_STATE_CONNECTING,
    HAL_BT_STATE_CONNECTED
} hal_bt_state_t;

/**
 * @brief Bluetooth data type enumeration
 */
typedef enum {
    HAL_BT_DATA_AUDIO,          /**< Audio stream data */
    HAL_BT_DATA_TELEMETRY,      /**< System telemetry data */
    HAL_BT_DATA_CONTROL,        /**< Control commands */
    HAL_BT_DATA_CONFIG          /**< Configuration data */
} hal_bt_data_type_t;

/**
 * @brief Bluetooth connection callback function type
 * @param connected true if connected, false if disconnected
 * @param user_data User-provided data pointer
 */
typedef void (*hal_bt_connection_callback_t)(bool connected, void *user_data);

/**
 * @brief Bluetooth data received callback function type
 * @param data_type Type of data received
 * @param data Pointer to received data
 * @param length Length of received data
 * @param user_data User-provided data pointer
 */
typedef void (*hal_bt_data_callback_t)(hal_bt_data_type_t data_type, 
                                       const uint8_t *data, size_t length, 
                                       void *user_data);

/**
 * @brief Bluetooth configuration structure
 */
typedef struct {
    char device_name[32];                          /**< Device name for advertising */
    uint16_t appearance;                           /**< Device appearance code */
    uint16_t max_mtu;                             /**< Maximum MTU size */
    hal_bt_connection_callback_t conn_callback;    /**< Connection state callback */
    hal_bt_data_callback_t data_callback;         /**< Data received callback */
    void *user_data;                              /**< User data for callbacks */
} hal_bt_config_t;

/**
 * @brief Bluetooth interface structure
 * 
 * This wraps around existing production transport.c functions to minimize changes.
 * Production code continues using transport.h APIs, while DevKit uses this HAL.
 */
typedef struct {
    /**
     * @brief Initialize Bluetooth subsystem
     * @param config Bluetooth configuration
     * @return 0 on success, negative on error
     */
    int (*init)(const hal_bt_config_t *config);
    
    /**
     * @brief Start advertising
     * @return 0 on success, negative on error
     */
    int (*start_advertising)(void);
    
    /**
     * @brief Stop advertising
     * @return 0 on success, negative on error
     */
    int (*stop_advertising)(void);
    
    /**
     * @brief Disconnect current connection
     * @return 0 on success, negative on error
     */
    int (*disconnect)(void);
    
    /**
     * @brief Send data over Bluetooth
     * @param data_type Type of data being sent
     * @param data Pointer to data buffer
     * @param length Length of data
     * @return 0 on success, negative on error
     */
    int (*send_data)(hal_bt_data_type_t data_type, const uint8_t *data, size_t length);
    
    /**
     * @brief Get current connection state
     * @return Current Bluetooth state
     */
    hal_bt_state_t (*get_state)(void);
    
    /**
     * @brief Get current MTU size
     * @return Current MTU in bytes, 0 if not connected
     */
    uint16_t (*get_mtu)(void);
    
    /**
     * @brief Get connection statistics
     * @param bytes_sent Output: total bytes sent
     * @param bytes_received Output: total bytes received  
     * @param packets_sent Output: total packets sent
     * @param packets_failed Output: total failed packet sends
     * @return 0 on success, negative on error
     */
    int (*get_stats)(uint32_t *bytes_sent, uint32_t *bytes_received, 
                     uint32_t *packets_sent, uint32_t *packets_failed);
    
} hal_bt_interface_t;

/**
 * @brief Get the platform-specific Bluetooth interface
 * @return Pointer to Bluetooth interface structure
 */
const hal_bt_interface_t* hal_bluetooth_get_interface(void);

/**
 * @brief Initialize Bluetooth HAL
 * This function sets up the platform-specific Bluetooth interface
 * @return 0 on success, negative on error
 */
int hal_bluetooth_init(void);

/* Convenience wrapper functions that delegate to the interface */
int hal_bluetooth_configure(const hal_bt_config_t *config);
int hal_bluetooth_start_advertising(void);
int hal_bluetooth_stop_advertising(void);
int hal_bluetooth_disconnect(void);
int hal_bluetooth_send_data(hal_bt_data_type_t data_type, const uint8_t *data, size_t length);
hal_bt_state_t hal_bluetooth_get_state(void);
uint16_t hal_bluetooth_get_mtu(void);
int hal_bluetooth_get_stats(uint32_t *bytes_sent, uint32_t *bytes_received, 
                           uint32_t *packets_sent, uint32_t *packets_failed);

/* Specialized send functions for common data types */
int hal_bluetooth_send_audio_packet(const uint8_t *data, size_t length);
int hal_bluetooth_send_telemetry(const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* HAL_BLUETOOTH_H */