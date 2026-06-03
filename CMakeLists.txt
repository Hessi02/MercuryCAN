cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_STANDARD 23)
set (CMAKE_CXX_STANDARD 23)

project(MercuryCAN)

set(CMAKE_C_FLAGS "-mmcu=at90can128 -Ofast -Wall") 
set(CMAKE_CXX_FLAGS "-mmcu=at90can128 -Ofast -Wall -fno-threadsafe-statics")
add_definitions(-DF_CPU=16000000UL -DBAUD=115200)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(MercuryCAN
            src/controller/driver.cpp 
            src/controller/transmitter.cpp 
            src/controller/receiver.cpp)