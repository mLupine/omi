# SPDX-License-Identifier: Apache-2.0
set(BOARD xiao_ble/nrf52840/sense)
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf52840_nordic)
enable_language(C ASM)

target_sources(app PRIVATE
    # DevKit-specific files
    src/main.c
    src/led.c
    src/lib/battery/battery.c
    src/speaker.c
    src/sdcard.c
    src/usb.c
    src/mic.c

    # Import core functionality from device/core (single source of truth)
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/codec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/transport.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/storage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/button.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/mic.c

    # Board-specific HAL implementation
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hal/nrf52840/mic_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hal/nrf52840/led_hal.c
)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

# Use shared Opus codec library
# Previously this file listed 130+ individual Opus source files
# Now consolidated to lib/opus-1.2.1/ with proper CMake library
if(CONFIG_OMI_CODEC_OPUS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/opus-1.2.1/ ${CMAKE_CURRENT_BINARY_DIR}/opus)
    target_link_libraries(app PRIVATE opus_codec)
endif()
