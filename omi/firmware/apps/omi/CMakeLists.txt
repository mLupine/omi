cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
enable_language(C ASM) # OPUS

project(omi)

file(GLOB app_sources
    src/main.c
    src/mic.c
    src/battery.c
    src/led.c
    src/haptic.c
    src/spi_flash.c
    src/settings.c
    src/feedback.c
)
file(GLOB core_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/codec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/transport.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/button.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/monitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/mic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/sd_card.c
)

if(CONFIG_OMI_ENABLE_OFFLINE_STORAGE)
    list(APPEND core_sources ${CMAKE_CURRENT_SOURCE_DIR}/../../device/core/storage.c)
endif()

file(GLOB hal_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hal/nrf5340/mic_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hal/nrf5340/led_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../hal/nrf5340/sd_hal.c
)

target_sources(app PRIVATE ${core_sources} ${app_sources} ${hal_sources})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)

if(CONFIG_OMI_CODEC_OPUS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../lib/opus-1.2.1/ ${CMAKE_CURRENT_BINARY_DIR}/opus)
    target_link_libraries(app PRIVATE opus_codec)
endif()
