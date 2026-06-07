cmake_minimum_required(VERSION 3.10)

project(MercuryCAN)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library(MercuryCAN
            src/controller/driver.cpp 
            src/controller/transmitter.cpp 
            src/controller/receiver.cpp
)