# Unified Omi Firmware

This directory contains the unified firmware build system that enables sharing code between production Omi hardware (nRF5340) and DevKit v2 hardware (nRF52840) with minimal changes to existing codebases.

## Overview

The unified firmware uses a Hardware Abstraction Layer (HAL) approach to provide a common interface while preserving existing production and DevKit functionality. This design minimizes changes to existing code while enabling maximum code reuse.

### Key Features

- **Minimal Production Code Changes**: Production firmware continues to work unchanged
- **DevKit v2 SDK Upgrade**: Upgrades DevKit v2 from SDK v2.7.0 to v2.9.0 for compatibility
- **Hardware Abstraction Layer**: Clean separation between platform-specific and shared code
- **Unified Build System**: Single CMake system supporting both platforms
- **Feature Flags**: Runtime and compile-time detection of platform capabilities
- **Legacy Compatibility**: Original build systems remain functional

## Architecture

```
unified/
├── src/
│   ├── hal/                    # Hardware Abstraction Layer interfaces
│   │   ├── hal_platform.h      # Platform detection and capabilities
│   │   ├── hal_audio.h         # Unified audio interface
│   │   └── hal_bluetooth.h     # Unified Bluetooth interface
│   ├── platform/               # Platform-specific implementations
│   │   ├── nrf5340/           # Production hardware (wraps existing code)
│   │   └── nrf52840/          # DevKit hardware (adapts existing code)
│   └── core/                   # Unified application code
├── configs/                    # Platform-specific configurations
├── overlays/                   # Device tree overlays
└── cmake/                      # Build system components
```

## Supported Platforms

### Production Hardware (nRF5340)
- **Platform**: OMI production device with nRF5340 dual-core processor
- **Build Target**: `omi-production-nrf5340`
- **Changes**: Minimal - HAL wraps around existing production code
- **Features**: Full feature set (WiFi, advanced audio, external flash, etc.)

### DevKit v2 (nRF52840)  
- **Platform**: Seeed Xiao BLE Sense with nRF52840 single-core processor
- **Build Target**: `devkit-v2-nrf52840`
- **Changes**: Upgraded to SDK v2.9.0, uses HAL for unified interface
- **Features**: Constrained feature set (USB, SD card, basic audio, etc.)

## Building

### Prerequisites

1. **nRF Connect SDK v2.9.0** installed and configured
2. **nRF Command Line Tools** for flashing
3. **CMake 3.20+** and **Ninja** build system

### Quick Start

```bash
# Clone the repository and navigate to unified firmware
cd omi/firmware/unified

# Build production firmware (unified)
west build -b omi/nrf5340/cpuapp -p --cmake-only -- -DCONF_FILE=configs/omi_nrf5340_unified.conf
west build

# Build DevKit v2 firmware (unified) 
west build -b xiao_ble_sense -p --cmake-only -- -DCONF_FILE=configs/devkit_nrf52840_v290.conf -DDTC_OVERLAY_FILE=overlays/xiao_ble_sense_devkitv2_unified.overlay
west build

# Flash to device
west flash
```

### Using CMake Presets (Recommended)

The unified build system includes CMake presets for easy building:

```bash
# List available presets
cmake --list-presets

# Configure and build production firmware
cmake --preset omi-production-nrf5340
cmake --build build/omi-production-nrf5340

# Configure and build DevKit v2 firmware
cmake --preset devkit-v2-nrf52840
cmake --build build/devkit-v2-nrf52840
```

### Available Build Targets

| Preset | Description | Platform | Changes |
|--------|-------------|----------|---------|
| `omi-production-nrf5340` | Production unified build | nRF5340 | Minimal - HAL overlay |
| `devkit-v2-nrf52840` | DevKit v2 unified build | nRF52840 | SDK upgrade + HAL |
| `omi-production-legacy` | Legacy production build | nRF5340 | None - original code |
| `devkit-v2-legacy` | Legacy DevKit build | nRF52840 | None - original code |

## Configuration

### Platform Detection

The build system automatically detects the target platform based on the board selection:

- `BOARD=omi/*` → Production nRF5340 platform
- `BOARD=xiao_*` → DevKit v2 nRF52840 platform

### Feature Flags

Platform capabilities are managed through feature flags:

```c
// Runtime feature detection
if (hal_platform_feature_available("wifi")) {
    // WiFi is available (production only)
}

if (hal_platform_feature_available("usb")) {
    // USB is available (DevKit v2 only)  
}

// Compile-time detection
#ifdef CONFIG_OMI_PLATFORM_NRF5340
    // Production-specific code
#elif CONFIG_OMI_PLATFORM_NRF52840
    // DevKit-specific code
#endif
```

### Memory Constraints

The build system automatically optimizes for platform constraints:

- **nRF5340**: Full feature set, larger buffers, dual-core optimizations
- **nRF52840**: Reduced feature set, smaller buffers, single-core optimizations

## HAL Interface Usage

### Audio Interface

```c
#include "hal/hal_audio.h"

// Configure audio
hal_audio_config_t config = {
    .sample_rate = HAL_AUDIO_SAMPLE_RATE_16KHZ,
    .format = HAL_AUDIO_FORMAT_PCM_S16LE,
    .channels = 1,
    .callback = audio_callback,
};

hal_audio_configure(&config);
hal_audio_start_recording();
```

### Bluetooth Interface

```c
#include "hal/hal_bluetooth.h"

// Configure Bluetooth
hal_bt_config_t config = {
    .device_name = "Omi Device",
    .conn_callback = connection_callback,
};

hal_bluetooth_configure(&config);
hal_bluetooth_start_advertising();
hal_bluetooth_send_audio_packet(data, length);
```

## Migration Status

### ✅ Completed (Phase 1)

- [x] HAL interface design
- [x] Platform detection system
- [x] Unified build system
- [x] Production nRF5340 HAL wrapper
- [x] DevKit nRF52840 HAL adaptation
- [x] CMake presets configuration
- [x] SDK v2.9.0 upgrade for DevKit

### 🚧 In Progress (Phase 2)

- [ ] Complete audio HAL implementation
- [ ] Complete Bluetooth HAL implementation  
- [ ] Storage HAL implementation
- [ ] Power management HAL
- [ ] Codec unification

### 📋 Planned (Phase 3-4)

- [ ] Monitoring system unification
- [ ] OTA update system
- [ ] Comprehensive testing
- [ ] Memory optimization for nRF52840
- [ ] Performance benchmarking

## Testing

### Unit Tests

```bash
# Run HAL unit tests
west build -t test

# Run platform-specific tests
west build -b omi/nrf5340/cpuapp -t test_production
west build -b xiao_ble_sense -t test_devkit
```

### Hardware Testing

1. **Audio Pipeline**: Test audio recording and streaming on both platforms
2. **Bluetooth Connectivity**: Verify BLE connections and data transfer
3. **Power Consumption**: Measure power usage on both platforms
4. **Feature Parity**: Ensure core features work identically

## Troubleshooting

### Build Issues

**Error**: `No such file or directory: configs/omi_nrf5340_unified.conf`
**Solution**: Ensure you're building from the `unified/` directory

**Error**: `BOARD not supported`
**Solution**: Verify board definition exists and BOARD_ROOT is set correctly

### Runtime Issues

**Problem**: Audio not working on DevKit v2
**Solution**: Check SDK v2.9.0 compatibility and PDM configuration

**Problem**: Memory allocation failures on nRF52840
**Solution**: Reduce buffer sizes in DevKit configuration

### Legacy Compatibility

The unified build maintains full compatibility with original build systems:

```bash
# Original production build (unchanged)
cd ../omi
west build -b omi/nrf5340/cpuapp

# Original DevKit build (unchanged)  
cd ../devkit
west build -b xiao_ble_sense
```

## Contributing

When adding new features to the unified firmware:

1. **Update HAL interfaces** if new hardware functionality is needed
2. **Implement platform-specific code** in respective platform directories
3. **Add feature flags** for platform capability differences
4. **Update configurations** for both platforms
5. **Test on both hardware platforms** before submitting

### Code Style

- Follow existing Zephyr coding standards
- Use HAL interfaces for platform abstraction
- Minimize changes to original production code
- Document platform-specific behavior

## Support

For issues related to:
- **Production firmware**: Existing production support channels
- **DevKit firmware**: Existing DevKit support channels  
- **Unified build system**: Create issue in repository with `unified-firmware` label

## Future Roadmap

The unified firmware enables:

1. **Faster Development**: Single codebase for new features
2. **Consistent Experience**: Same functionality across device tiers
3. **Easier Maintenance**: Reduced code duplication
4. **Better Testing**: Unified test infrastructure
5. **Simplified CI/CD**: Single build and release pipeline

This unified approach provides the foundation for a scalable, maintainable firmware ecosystem supporting current and future Omi hardware variants.