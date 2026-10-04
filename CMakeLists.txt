cmake_minimum_required(VERSION 3.10)

project(MercuryCAN)

if(MOCK_SOURCES)
    add_library(MercuryCAN
                src/driver/driver.cpp 
                src/test/mockDriver.cpp
                src/controller/transmitter.cpp 
                src/controller/receiver.cpp
    )
else()
    add_library(MercuryCAN
                src/driver/driver.cpp 
                src/driver/avrDriver.cpp
                src/controller/transmitter.cpp 
                src/controller/receiver.cpp
    )
endif()

target_include_directories(MercuryCAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)