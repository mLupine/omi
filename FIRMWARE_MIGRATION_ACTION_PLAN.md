# Firmware Migration Action Plan: Production to DevKit v2 Unification

## Overview
This document provides the detailed technical roadmap for unifying the production Omi firmware with the DevKit v2 codebase to create a single, maintainable firmware solution supporting both hardware platforms.

## Phase 1: Foundation and Preparation (Weeks 1-6)

### 1.1 SDK Version Harmonization (Week 1-2)

**Objective**: Upgrade DevKit v2 to use nRF Connect SDK v2.9.0 to match production firmware.

**Tasks**:
```bash
# Current: DevKit v2 uses SDK v2.7.0
# Target: Upgrade to SDK v2.9.0

1. Update DevKit v2 CMakePresets.json
   - Modify toolchain requirements
   - Update board configurations
   
2. Resolve API changes between v2.7.0 and v2.9.0
   - Bluetooth API updates
   - Audio driver changes
   - GPIO and peripheral APIs
   
3. Update configuration files
   - prj_xiao_ble_sense_devkitv2-adafruit.conf
   - Overlay files for new SDK version
   
4. Validate build and basic functionality
   - Ensure DevKit v2 builds with new SDK
   - Test basic BLE connectivity
   - Verify audio recording functionality
```

**Deliverables**:
- Updated DevKit v2 project files for SDK v2.9.0
- Migration notes documenting API changes
- Validated build for DevKit v2 with new SDK

### 1.2 Hardware Abstraction Layer Design (Week 2-3)

**Objective**: Design and implement a Hardware Abstraction Layer (HAL) to isolate platform-specific code.

**HAL Structure**:
```c
// hal/hal_types.h - Common type definitions
typedef enum {
    PLATFORM_OMI_NRF5340,
    PLATFORM_XIAO_NRF52840
} hal_platform_t;

typedef struct {
    hal_platform_t platform;
    uint32_t cpu_freq;
    uint32_t ram_size;
    uint32_t flash_size;
    bool dual_core_support;
} hal_platform_info_t;

// hal/hal_audio.h - Audio abstraction
typedef struct {
    int (*init)(void);
    int (*start_recording)(hal_audio_callback_t callback);
    int (*stop_recording)(void);
    int (*set_sample_rate)(uint32_t sample_rate);
} hal_audio_interface_t;

// hal/hal_bluetooth.h - Bluetooth abstraction
typedef struct {
    int (*init)(void);
    int (*start_advertising)(void);
    int (*send_data)(uint8_t *data, size_t len);
} hal_bluetooth_interface_t;

// hal/hal_storage.h - Storage abstraction
typedef struct {
    int (*init)(void);
    int (*write)(uint32_t offset, uint8_t *data, size_t len);
    int (*read)(uint32_t offset, uint8_t *data, size_t len);
    int (*erase)(uint32_t offset, size_t len);
} hal_storage_interface_t;
```

**Implementation Plan**:
```
src/hal/
├── hal_platform.h        # Platform detection and info
├── hal_audio.h           # Audio interface abstraction
├── hal_bluetooth.h       # Bluetooth interface abstraction
├── hal_storage.h         # Storage interface abstraction
├── hal_power.h           # Power management abstraction
├── nrf5340/             # Production platform implementation
│   ├── hal_nrf5340_audio.c
│   ├── hal_nrf5340_bluetooth.c
│   ├── hal_nrf5340_storage.c
│   └── hal_nrf5340_power.c
└── nrf52840/            # DevKit platform implementation
    ├── hal_nrf52840_audio.c
    ├── hal_nrf52840_bluetooth.c
    ├── hal_nrf52840_storage.c
    └── hal_nrf52840_power.c
```

### 1.3 Unified Build System (Week 3-4)

**Objective**: Create a single build system that can target both platforms with conditional compilation.

**CMakeLists.txt Structure**:
```cmake
# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.20.0)

# Platform detection
if(BOARD MATCHES "omi")
    set(OMI_PLATFORM "nrf5340")
    set(OMI_PLATFORM_NRF5340 TRUE)
elseif(BOARD MATCHES "xiao")
    set(OMI_PLATFORM "nrf52840") 
    set(OMI_PLATFORM_NRF52840 TRUE)
else()
    message(FATAL_ERROR "Unsupported board: ${BOARD}")
endif()

# Common source files
file(GLOB_RECURSE COMMON_SOURCES
    "src/core/*.c"
    "src/lib/*.c"
)

# Platform-specific sources
if(OMI_PLATFORM_NRF5340)
    file(GLOB_RECURSE PLATFORM_SOURCES "src/platform/nrf5340/*.c")
    file(GLOB_RECURSE HAL_SOURCES "src/hal/nrf5340/*.c")
elseif(OMI_PLATFORM_NRF52840)
    file(GLOB_RECURSE PLATFORM_SOURCES "src/platform/nrf52840/*.c")
    file(GLOB_RECURSE HAL_SOURCES "src/hal/nrf52840/*.c")
endif()

target_sources(app PRIVATE 
    ${COMMON_SOURCES}
    ${PLATFORM_SOURCES}
    ${HAL_SOURCES}
)

# Platform-specific configuration
target_compile_definitions(app PRIVATE
    -DOMI_PLATFORM_${OMI_PLATFORM}
    -DOMI_PLATFORM="${OMI_PLATFORM}"
)
```

**CMakePresets.json**:
```json
{
    "version": 2,
    "configurePresets": [
        {
            "name": "omi-nrf5340",
            "displayName": "Omi Production (nRF5340)",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/omi-nrf5340",
            "cacheVariables": {
                "BOARD": "omi/nrf5340/cpuapp",
                "CONF_FILE": "${sourceDir}/configs/omi_nrf5340.conf"
            }
        },
        {
            "name": "devkit-nrf52840",
            "displayName": "DevKit v2 (nRF52840)",
            "generator": "Ninja", 
            "binaryDir": "${sourceDir}/build/devkit-nrf52840",
            "cacheVariables": {
                "BOARD": "xiao_ble_sense",
                "CONF_FILE": "${sourceDir}/configs/devkit_nrf52840.conf",
                "DTC_OVERLAY_FILE": "${sourceDir}/overlays/xiao_ble_sense_devkitv2.overlay"
            }
        }
    ]
}
```

### 1.4 Feature Flag System (Week 4-5)

**Objective**: Implement comprehensive feature flags to handle platform capability differences.

**Feature Configuration System**:
```c
// src/core/features.h
typedef struct {
    bool wifi_enabled;
    bool dual_core_available;
    bool external_flash_available;
    bool usb_support;
    bool advanced_audio_processing;
    bool hardware_crypto;
    uint32_t max_ble_mtu;
    uint32_t audio_buffer_size;
    uint32_t storage_capacity;
} omi_feature_config_t;

// Platform-specific feature definitions
#ifdef OMI_PLATFORM_NRF5340
#define OMI_FEATURES_DEFAULT { \
    .wifi_enabled = true, \
    .dual_core_available = true, \
    .external_flash_available = true, \
    .usb_support = false, \
    .advanced_audio_processing = true, \
    .hardware_crypto = true, \
    .max_ble_mtu = 498, \
    .audio_buffer_size = 4096, \
    .storage_capacity = 0x1E000000 \
}
#endif

#ifdef OMI_PLATFORM_NRF52840
#define OMI_FEATURES_DEFAULT { \
    .wifi_enabled = false, \
    .dual_core_available = false, \
    .external_flash_available = false, \
    .usb_support = true, \
    .advanced_audio_processing = false, \
    .hardware_crypto = false, \
    .max_ble_mtu = 498, \
    .audio_buffer_size = 2048, \
    .storage_capacity = 0xFFFF0000 \
}
#endif

// Runtime feature detection
const omi_feature_config_t* omi_get_features(void);
bool omi_feature_enabled(omi_feature_t feature);
```

### 1.5 Unified Configuration Management (Week 5-6)

**Objective**: Consolidate configuration files and create platform-specific variants.

**Configuration Structure**:
```
configs/
├── common.conf              # Shared configuration
├── omi_nrf5340.conf        # Production-specific config
├── devkit_nrf52840.conf    # DevKit-specific config
└── features/               # Feature-specific configs
    ├── audio.conf
    ├── bluetooth.conf  
    ├── storage.conf
    └── power.conf
```

**Configuration Includes**:
```bash
# configs/omi_nrf5340.conf
include configs/common.conf
include configs/features/audio.conf
include configs/features/bluetooth.conf
include configs/features/storage.conf

# nRF5340 specific configurations
CONFIG_BOARD_OMI_NRF5340_CPUAPP=y
CONFIG_OMI_ENABLE_WIFI=y
CONFIG_OMI_DUAL_CORE=y
CONFIG_OMI_EXTERNAL_FLASH=y

# configs/devkit_nrf52840.conf  
include configs/common.conf
include configs/features/audio.conf
include configs/features/bluetooth.conf
include configs/features/storage.conf

# nRF52840 specific configurations
CONFIG_BOARD_XIAO_BLE_SENSE=y
CONFIG_OMI_ENABLE_USB=y
CONFIG_OMI_SINGLE_CORE=y
CONFIG_OMI_SD_CARD_ONLY=y
```

## Phase 2: Core Unification (Weeks 7-14)

### 2.1 Transport Layer Unification (Week 7-8)

**Objective**: Merge the Bluetooth transport implementations into a unified layer.

**Unified Transport Interface**:
```c
// src/core/transport/transport.h
typedef struct {
    int (*init)(void);
    int (*start_advertising)(const char* device_name);
    int (*stop_advertising)(void);
    int (*send_audio_data)(uint8_t *data, size_t len);
    int (*send_sensor_data)(uint8_t *data, size_t len);
    int (*register_connection_callback)(transport_connection_callback_t cb);
    int (*register_data_callback)(transport_data_callback_t cb);
} transport_interface_t;

// Unified implementation with platform-specific backends
int transport_init(void);
int transport_send_audio_packet(uint8_t *data, size_t len);
int transport_send_telemetry(const telemetry_data_t *data);
```

**Migration Tasks**:
```c
1. Extract common BLE GATT service definitions
2. Unify audio data streaming protocol
3. Consolidate connection management
4. Merge MTU negotiation logic
5. Standardize error handling
6. Create unified callback system
```

### 2.2 Audio System Unification (Week 8-10)

**Objective**: Create unified audio processing pipeline that works on both single and dual-core platforms.

**Audio Architecture**:
```c
// src/core/audio/audio_manager.h
typedef struct {
    bool (*init)(uint32_t sample_rate, uint8_t channels);
    bool (*start_recording)(audio_callback_t callback);
    bool (*stop_recording)(void);
    bool (*set_gain)(float gain);
    uint32_t (*get_buffer_size)(void);
} audio_manager_interface_t;

// Platform-specific optimizations
#ifdef OMI_PLATFORM_NRF5340
    // Use network core for audio preprocessing
    // Hardware PDM with DMA optimizations
#endif

#ifdef OMI_PLATFORM_NRF52840
    // Single-core audio processing
    // Software-based optimizations
#endif
```

### 2.3 Codec Unification (Week 10-11)

**Objective**: Merge the Opus codec implementations and create unified audio compression pipeline.

**Tasks**:
```c
1. Consolidate Opus library versions
   - Currently: Production uses subdirectory, DevKit uses inline files
   - Target: Single Opus implementation with build system selection

2. Unify codec configuration
   - Sample rates, bit rates, frame sizes
   - Platform-specific optimizations

3. Create codec manager
   - Abstract codec selection (Opus/PCM)
   - Handle platform memory constraints
   - Optimize for real-time performance
```

### 2.4 Storage Layer Unification (Week 11-12)

**Objective**: Create abstracted storage layer supporting SPI Flash, SD cards, and internal flash.

**Storage Abstraction**:
```c
// src/core/storage/storage_manager.h
typedef enum {
    STORAGE_TYPE_INTERNAL,
    STORAGE_TYPE_EXTERNAL_FLASH,
    STORAGE_TYPE_SD_CARD
} storage_type_t;

typedef struct {
    storage_type_t type;
    uint32_t capacity;
    uint32_t block_size;
    bool (*init)(void);
    bool (*read)(uint32_t addr, void *data, size_t len);
    bool (*write)(uint32_t addr, const void *data, size_t len);
    bool (*erase)(uint32_t addr, size_t len);
} storage_backend_t;

// Platform-specific storage configuration
#ifdef OMI_PLATFORM_NRF5340
    // SPI Flash + SD Card support
    // Multi-tier storage management
#endif

#ifdef OMI_PLATFORM_NRF52840  
    // SD Card primary storage
    // Internal flash for critical data
#endif
```

### 2.5 Power Management Unification (Week 12-13)

**Objective**: Create unified power management supporting both dual-core and single-core architectures.

**Power Management Interface**:
```c
// src/core/power/power_manager.h
typedef enum {
    POWER_MODE_ACTIVE,
    POWER_MODE_LOW_POWER,
    POWER_MODE_SLEEP,
    POWER_MODE_DEEP_SLEEP
} power_mode_t;

typedef struct {
    bool (*init)(void);
    bool (*set_mode)(power_mode_t mode);
    uint32_t (*get_battery_level)(void);
    bool (*is_charging)(void);
    bool (*enable_wakeup_source)(wakeup_source_t source);
} power_manager_interface_t;
```

### 2.6 Peripheral Management Unification (Week 13-14)

**Objective**: Unify LED, button, haptic, and sensor management.

**Peripheral Interfaces**:
```c
// src/core/peripherals/
├── led_manager.h/.c        # Unified LED control
├── button_manager.h/.c     # Button handling
├── haptic_manager.h/.c     # Haptic feedback
├── sensor_manager.h/.c     # Accelerometer/sensors
└── battery_manager.h/.c    # Battery monitoring
```

## Phase 3: Feature Harmonization (Weeks 15-20)

### 3.1 Monitoring and Telemetry (Week 15-16)

**Objective**: Port the production monitoring system to work on DevKit v2 with appropriate optimizations.

**Unified Monitoring System**:
```c
// src/core/monitoring/monitor.h
typedef struct {
    uint32_t audio_packets_sent;
    uint32_t audio_packets_failed;
    uint32_t ble_connections;
    uint32_t ble_disconnections;
    uint32_t battery_level;
    uint32_t storage_usage;
    uint32_t cpu_usage;         // Platform-specific
    uint32_t memory_usage;      // Platform-specific
} system_metrics_t;

// Platform-specific metric collection
#ifdef OMI_PLATFORM_NRF5340
    // Dual-core CPU monitoring
    // Advanced memory tracking
#endif

#ifdef OMI_PLATFORM_NRF52840
    // Single-core monitoring
    // Simplified metrics for resource constraints
#endif
```

### 3.2 OTA Update System (Week 16-17)

**Objective**: Implement unified OTA update capability for both platforms.

**OTA Architecture**:
```c
// src/core/ota/ota_manager.h
typedef struct {
    bool (*init)(void);
    bool (*start_update)(uint32_t image_size);
    bool (*write_chunk)(uint32_t offset, uint8_t *data, size_t len);
    bool (*verify_image)(void);
    bool (*apply_update)(void);
    ota_status_t (*get_status)(void);
} ota_manager_interface_t;

// Platform-specific implementations
#ifdef OMI_PLATFORM_NRF5340
    // MCUboot with dual-core support
    // Network core update handling
#endif

#ifdef OMI_PLATFORM_NRF52840
    // Standard MCUboot integration
    // Single-image update process  
#endif
```

### 3.3 User Interface Unification (Week 17-18)

**Objective**: Harmonize user feedback systems (LED patterns, haptic feedback, audio cues).

### 3.4 Security Model Alignment (Week 18-19)

**Objective**: Implement consistent security across platforms while respecting hardware limitations.

### 3.5 Configuration and Settings Management (Week 19-20)

**Objective**: Unify persistent configuration storage and runtime settings management.

## Phase 4: Optimization and Validation (Weeks 21-26)

### 4.1 Memory Optimization for nRF52840 (Week 21-22)

**Objective**: Optimize memory usage for DevKit v2's resource constraints.

**Optimization Strategies**:
```c
1. Code size reduction:
   - Remove unused features via feature flags
   - Optimize library inclusions
   - Use link-time optimization (LTO)

2. RAM optimization:
   - Reduce buffer sizes where appropriate
   - Implement memory pooling
   - Optimize data structures

3. Flash optimization:
   - Compress audio codecs
   - Remove debug symbols in release builds
   - Optimize constant data storage
```

### 4.2 Performance Tuning (Week 22-23)

**Objective**: Optimize performance for single-core DevKit v2 while maintaining production quality.

### 4.3 Comprehensive Testing (Week 23-24)

**Objective**: Validate functionality across both platforms with automated testing.

**Testing Strategy**:
```
1. Unit Tests:
   - HAL layer functionality
   - Core business logic
   - Platform-specific implementations

2. Integration Tests:
   - Cross-platform communication
   - Audio pipeline end-to-end
   - Storage operations

3. Hardware-in-the-Loop Tests:
   - Real device validation
   - Bluetooth connectivity
   - Audio quality assessment
   - Power consumption measurement

4. Regression Tests:
   - Ensure existing functionality
   - Performance benchmarks
   - Memory usage validation
```

### 4.4 Documentation and Developer Experience (Week 24-25)

**Objective**: Create comprehensive documentation for the unified codebase.

### 4.5 CI/CD Pipeline Implementation (Week 25-26)

**Objective**: Establish automated build and test pipeline for both platforms.

**Pipeline Structure**:
```yaml
# .github/workflows/build-test.yml
name: Build and Test Unified Firmware

on: [push, pull_request]

jobs:
  build-omi-nrf5340:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup nRF Connect SDK v2.9.0
      - name: Build Production Firmware
        run: west build -b omi/nrf5340/cpuapp -p
      - name: Run Tests
        run: pytest tests/test_production.py

  build-devkit-nrf52840:
    runs-on: ubuntu-latest  
    steps:
      - uses: actions/checkout@v3
      - name: Setup nRF Connect SDK v2.9.0
      - name: Build DevKit Firmware
        run: west build -b xiao_ble_sense -p
      - name: Run Tests
        run: pytest tests/test_devkit.py
        
  integration-tests:
    needs: [build-omi-nrf5340, build-devkit-nrf52840]
    runs-on: ubuntu-latest
    steps:
      - name: Run Cross-Platform Tests
        run: pytest tests/test_integration.py
```

## Success Metrics and Validation

### Technical Metrics
- **Build Success**: Both platforms build without errors
- **Memory Usage**: DevKit v2 firmware fits within nRF52840 constraints
- **Performance**: Audio latency within acceptable limits
- **Code Coverage**: >80% test coverage for unified codebase
- **Code Reduction**: >50% reduction in duplicated code

### Functional Metrics
- **Feature Parity**: Core features work on both platforms
- **Audio Quality**: No degradation in audio processing quality
- **Bluetooth Stability**: Reliable BLE connectivity and data transfer
- **Power Consumption**: Acceptable battery life on both platforms
- **OTA Updates**: Successful firmware updates on both platforms

## Risk Mitigation

### Technical Risks
1. **Memory Constraints**: Continuous monitoring with automated alerts
2. **Performance Issues**: Early prototyping and benchmarking
3. **SDK Incompatibilities**: Gradual migration with fallback plans

### Process Risks
1. **Schedule Delays**: Buffer time built into each phase
2. **Resource Availability**: Cross-training team members
3. **Quality Issues**: Comprehensive testing at each milestone

## Conclusion

This action plan provides a structured approach to unifying the production and DevKit v2 firmware codebases. The phased approach allows for incremental progress with validation at each step, ensuring that both platforms remain functional throughout the migration process.

The key success factors are:
1. **Strong hardware abstraction** to isolate platform differences
2. **Comprehensive feature flags** to handle capability variations  
3. **Thorough testing** to ensure quality across platforms
4. **Careful memory management** to work within nRF52840 constraints
5. **Clear documentation** to support future development

Upon completion, the unified codebase will provide significant benefits in terms of maintainability, development velocity, and feature consistency across the Omi device ecosystem.