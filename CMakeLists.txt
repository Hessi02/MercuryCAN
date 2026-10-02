cmake_minimum_required(VERSION 3.10)

project(MercuryCAN)

add_library(MercuryCAN
            src/controller/driver.cpp 
            src/controller/avrDriver.cpp
            src/controller/transmitter.cpp 
            src/controller/receiver.cpp
)

target_include_directories(MercuryCAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)