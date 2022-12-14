# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/pi/pico/pico-sdk")

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PiPicoILI9341 C CXX ASM)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(PiPicoILI9341
        PiPicoILI9341.cpp
        display.cpp
        commands.c)

pico_set_program_name(PiPicoILI9341 "PiPicoILI9341")
pico_set_program_version(PiPicoILI9341 "0.1")

pico_enable_stdio_uart(PiPicoILI9341 1)
pico_enable_stdio_usb(PiPicoILI9341 0)

# Add the standard library to the build
target_link_libraries(PiPicoILI9341 pico_stdlib)

# Add any user requested libraries
target_link_libraries(PiPicoILI9341
        hardware_spi
        )

pico_add_extra_outputs(PiPicoILI9341)

