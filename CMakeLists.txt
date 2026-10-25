cmake_minimum_required(VERSION 3.10)

project(MercuryCAN)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("${CMAKE_CURRENT_SOURCE_DIR}/res/cmake/embed_dbc.cmake")

if(MOCK_SOURCES)
    find_package(Threads REQUIRED)

    add_library(MercuryCAN
                src/driver/driver.cpp 
                src/driver/mockDriver.cpp
                src/controller/transmitter.cpp 
                src/controller/receiver.cpp
    )
    target_compile_definitions(MercuryCAN PRIVATE __USE_MOCK_DRIVER)
    target_link_libraries(MercuryCAN PRIVATE Threads::Threads)
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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)

target_include_directories(MercuryCAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)