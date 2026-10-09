cmake_minimum_required(VERSION 3.10)

project(MercuryCAN)

if(MOCK_SOURCES)
    add_library(MercuryCAN
                src/driver/driver.cpp 
                src/test/mockDriver.cpp
                src/controller/transmitter.cpp 
                src/controller/receiver.cpp
    )
    target_compile_definitions(MercuryCAN PRIVATE __USE_MOCK_DRIVER)
else()
    add_library(MercuryCAN
                src/driver/driver.cpp 
                src/driver/avrDriver.cpp
                src/controller/transmitter.cpp 
                src/controller/receiver.cpp
    )
endif()

enable_testing()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

target_include_directories(MercuryCAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)