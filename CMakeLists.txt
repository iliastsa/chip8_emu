cmake_minimum_required(VERSION 3.1.0)
project(my_chip8_emu)

include_directories(include)

set(CMAKE_CXX_STANDARD 11)
set(SOURCES src/main.cpp src/chip8.cpp)

add_executable(chip8_emu ${SOURCES})
