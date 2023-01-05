cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(bms-monitor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

add_executable(bms-monitor bms-monitor.cpp Nextion.cpp Nextion.h SmartBMS.cpp SmartBMS.h BMSDisplay.cpp BMSDisplay.h)

pico_add_extra_outputs(bms-monitor)

target_link_libraries(bms-monitor pico_stdlib)
