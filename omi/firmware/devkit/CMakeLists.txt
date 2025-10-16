# SPDX-License-Identifier: Apache-2.0
set(BOARD xiao_ble/nrf52840/sense)
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf52840_nordic)
enable_language(C ASM)

# Define production core directory as source of truth
set(PROD_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../omi/src/lib/core)

target_sources(app PRIVATE
    # DevKit-specific files remain in src/
    src/main.c
    src/transport.c
    src/mic.c
    src/led.c
    src/lib/battery/battery.c
    src/speaker.c
    src/sdcard.c
    src/usb.c
    src/storage.c

    # Import core functionality from production
    ${PROD_CORE_DIR}/codec.c
    # Note: button.c and storage.c import deferred - need header compatibility layer
	# src/nfc.c future release
)

# Use shared Opus codec library
# Previously this file listed 130+ individual Opus source files
# Now consolidated to shared/lib/opus-1.2.1/ with proper CMake library
if(CONFIG_OMI_CODEC_OPUS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared/lib/opus-1.2.1/ ${CMAKE_CURRENT_BINARY_DIR}/opus)
    target_link_libraries(app PRIVATE opus_codec)
endif()
