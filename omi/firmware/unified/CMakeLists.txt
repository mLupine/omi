# Unified Omi Firmware CMakeLists.txt
# This build system unifies production and DevKit v2 firmware with minimal changes to existing code

cmake_minimum_required(VERSION 3.20.0)

# Platform detection based on board selection
if(BOARD MATCHES "omi")
    set(OMI_PLATFORM "nrf5340")
    set(OMI_PLATFORM_NRF5340 TRUE)
    message(STATUS "Building for Production hardware (nRF5340)")
elseif(BOARD MATCHES "xiao")
    set(OMI_PLATFORM "nrf52840") 
    set(OMI_PLATFORM_NRF52840 TRUE)
    message(STATUS "Building for DevKit v2 hardware (nRF52840)")
else()
    message(FATAL_ERROR "Unsupported board: ${BOARD}")
endif()

# Set up Zephyr
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
enable_language(C ASM) # OPUS requires ASM

# Project definition
project(omi-unified)

# Define unified build flag
add_compile_definitions(CONFIG_OMI_UNIFIED_BUILD=1)
add_compile_definitions(OMI_PLATFORM_${OMI_PLATFORM}=1)

# HAL source files (always included)
file(GLOB HAL_SOURCES
    "src/hal/*.c"
)

# Platform-specific HAL implementation
if(OMI_PLATFORM_NRF5340)
    file(GLOB PLATFORM_HAL_SOURCES "src/platform/nrf5340/*.c")
elseif(OMI_PLATFORM_NRF52840)
    file(GLOB PLATFORM_HAL_SOURCES "src/platform/nrf52840/*.c")
endif()

# Include existing production code with minimal changes
if(OMI_PLATFORM_NRF5340)
    message(STATUS "Including production firmware sources")
    
    # Include existing production source files
    file(GLOB PRODUCTION_SOURCES
        "../omi/src/main.c"
        "../omi/src/mic.c"
        "../omi/src/battery.c"
        "../omi/src/led.c"
        "../omi/src/haptic.c"
        "../omi/src/sd_card.c"
        "../omi/src/spi_flash.c"
        "../omi/src/settings.c"
        "../omi/src/feedback.c"
    )
    
    # Include existing production core library
    file(GLOB PRODUCTION_CORE_SOURCES
        "../omi/src/lib/core/codec.c"
        "../omi/src/lib/core/transport.c"
        "../omi/src/lib/core/button.c"
        "../omi/src/lib/core/monitor.c"
    )
    
    # Conditional inclusion of storage if enabled
    if(CONFIG_OMI_ENABLE_OFFLINE_STORAGE)
        list(APPEND PRODUCTION_CORE_SOURCES "../omi/src/lib/core/storage.c")
    endif()
    
    # Add production sources to build
    target_sources(app PRIVATE ${PRODUCTION_SOURCES} ${PRODUCTION_CORE_SOURCES})
    
    # Include production directories for headers
    target_include_directories(app PRIVATE
        "../omi/src"
        "../omi/src/lib/core"
    )
    
    # Use production configuration
    set(CONF_FILE "../omi/omi.conf")
    
elseif(OMI_PLATFORM_NRF52840)
    message(STATUS "Including DevKit v2 firmware sources")
    
    # Include existing DevKit sources (adapted to unified HAL)
    file(GLOB DEVKIT_SOURCES
        "src/core/main_unified.c"        # New unified main that uses HAL
        "../devkit/src/transport.c"      # Existing DevKit transport
        "../devkit/src/codec.c"          # Existing DevKit codec
        "../devkit/src/led.c"            # Existing DevKit LED
        "../devkit/src/button.c"         # Existing DevKit button
        "../devkit/src/speaker.c"        # Existing DevKit speaker
        "../devkit/src/sdcard.c"         # Existing DevKit SD card
        "../devkit/src/storage.c"        # Existing DevKit storage
        "../devkit/src/usb.c"            # Existing DevKit USB
    )
    
    # Add DevKit sources to build  
    target_sources(app PRIVATE ${DEVKIT_SOURCES})
    
    # Include DevKit directories for headers
    target_include_directories(app PRIVATE
        "../devkit/src"
        "../devkit/src/lib"
    )
    
    # Use updated DevKit configuration (upgraded to SDK v2.9.0)
    set(CONF_FILE "configs/devkit_nrf52840_v290.conf")
    
endif()

# Add HAL sources (always included)
target_sources(app PRIVATE ${HAL_SOURCES} ${PLATFORM_HAL_SOURCES})

# Include HAL headers for all builds
target_include_directories(app PRIVATE
    "src/hal"
    "src/platform"
)

# Opus codec handling - unify between platforms
if(CONFIG_OMI_CODEC_OPUS)
    if(OMI_PLATFORM_NRF5340)
        # Use existing production Opus build system
        add_subdirectory(../omi/src/lib/core/lib/opus-1.2.1/ opus_production)
        target_link_libraries(app PRIVATE opus_codec)
    elseif(OMI_PLATFORM_NRF52840)
        # Use existing DevKit Opus sources (inline files)
        # This maintains compatibility with existing DevKit build
        include(cmake/DevKitOpus.cmake)
    endif()
endif()

# Platform-specific compiler flags
if(OMI_PLATFORM_NRF52840)
    # DevKit-specific Opus flags (from existing DevKit CMakeLists.txt)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DARM_MATH_CM4")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPUS_ARM_ASM")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPUS_ARM_INLINE_ASM")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPUS_ARM_INLINE_EDSP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPUS_ARM_INLINE_MEDIA")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPUS_ARM_MAY_HAVE_EDSP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPUS_ARM_PRESUME_EDSP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DOPUS_BUILD")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_ALLOCA")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFIXED_POINT -DDISABLE_FLOAT_API")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_CONFIG_H")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_ALLOCA_H")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsingle-precision-constant")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LRINT -DHAVE_LRINTF")
endif()

# Generate build info
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build_info.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/build_info.h"
)
target_include_directories(app PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

message(STATUS "Unified build configured for platform: ${OMI_PLATFORM}")
message(STATUS "Board: ${BOARD}")
message(STATUS "Configuration: ${CONF_FILE}")