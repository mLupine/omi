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

# Define unified build flags as compiler definitions
add_compile_definitions(CONFIG_OMI_UNIFIED_BUILD=1)
add_compile_definitions(CONFIG_OMI_PLATFORM_${OMI_PLATFORM}=1)
add_compile_definitions(OMI_PLATFORM_${OMI_PLATFORM}=1)

# Define HAL support
add_compile_definitions(CONFIG_OMI_HAL=1)
add_compile_definitions(CONFIG_OMI_HAL_AUDIO=1)
add_compile_definitions(CONFIG_OMI_HAL_BLUETOOTH=1)
add_compile_definitions(CONFIG_OMI_HAL_PLATFORM=1)

# HAL source files - only header files in hal directory, implementations are in platform directories
# No .c files in hal directory, only headers

# Platform-specific HAL implementation
if(OMI_PLATFORM_NRF5340)
    file(GLOB PLATFORM_HAL_SOURCES "src/platform/nrf5340/*.c")
elseif(OMI_PLATFORM_NRF52840)
    file(GLOB PLATFORM_HAL_SOURCES "src/platform/nrf52840/*.c")
endif()

# Include existing production code with minimal changes
if(OMI_PLATFORM_NRF5340)
    message(STATUS "Including production firmware sources")
    
    # For now, use a minimal main that demonstrates HAL integration
    # Later we'll integrate full production sources
    target_sources(app PRIVATE
        "src/core/main_production_unified.c"
    )
    
    # Include production directories for headers (when needed)
    target_include_directories(app PRIVATE
        "../omi/src"
        "../omi/src/lib/core"
    )
    
    # Use unified production configuration  
    # Note: CONF_FILE is set via CMake presets
    
elseif(OMI_PLATFORM_NRF52840)
    message(STATUS "Including DevKit v2 firmware sources")
    
    # For now, use the unified main that demonstrates HAL integration
    target_sources(app PRIVATE
        "src/core/main_unified.c"        # New unified main that uses HAL
    )
    
    # Include DevKit directories for headers (when needed)
    target_include_directories(app PRIVATE
        "../devkit/src"
        "../devkit/src/lib"
    )
    
    # Use updated DevKit configuration (upgraded to SDK v2.9.0)
    # Note: CONF_FILE is set via CMake presets
    
endif()

# Add HAL sources (platform-specific implementations only)
target_sources(app PRIVATE ${PLATFORM_HAL_SOURCES})

# Include HAL headers for all builds
target_include_directories(app PRIVATE
    "src/hal"
    "src/platform"
)

# Opus codec handling - temporarily disabled to fix builds
# TODO: Re-enable once basic builds are working
# if(CONFIG_OMI_CODEC_OPUS)
#     if(OMI_PLATFORM_NRF5340)
#         # Use existing production Opus build system
#         add_subdirectory(../omi/src/lib/core/lib/opus-1.2.1/ opus_production)
#         target_link_libraries(app PRIVATE opus_codec)
#     elseif(OMI_PLATFORM_NRF52840)
#         # Use existing DevKit Opus sources (inline files)
#         # This maintains compatibility with existing DevKit build
#         include(cmake/DevKitOpus.cmake)
#     endif()
# endif()

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