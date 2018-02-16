cmake_minimum_required(VERSION 3.1.0)
project(my_chip8_emu)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)

include_directories(include ${SDL2_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(SOURCES src/main.cpp src/chip8.cpp)

add_executable(chip8_emu ${SOURCES})

target_link_libraries(chip8_emu ${SDL2_LIBRARY})
