# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(usec-133-rp2040-spi-example C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
include_directories(${PROJECT_SOURCE_DIR}/usec-133-spi-lib)
add_executable(usec-133-rp2040-spi-example main.c platform.c usec-133-spi-lib/usec_dev.c)

pico_set_program_name(usec-133-rp2040-spi-example "usec-133-rp2040-spi-example")
pico_set_program_version(usec-133-rp2040-spi-example "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(usec-133-rp2040-spi-example 0)
pico_enable_stdio_usb(usec-133-rp2040-spi-example 1)

# Add the standard library to the build
target_link_libraries(usec-133-rp2040-spi-example
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(usec-133-rp2040-spi-example PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(usec-133-rp2040-spi-example 
        hardware_spi
        )

pico_add_extra_outputs(usec-133-rp2040-spi-example)

