cmake_minimum_required(VERSION 3.10)

project(MercuryCAN)

if(MOCK_SOURCES)
    add_library(MercuryCAN
                src/driver/driver.cpp 
                src/test/mockDriver.cpp
                src/controller/transmitter.cpp 
                src/controller/receiver.cpp
    )

    enable_testing()

    add_executable(rxNoise test/mock/rxNoise/rxNoise.cpp)
    target_link_libraries(rxNoise PRIVATE MercuryCAN)
    target_include_directories(rxNoise PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

    add_test(
        NAME rxNoise
        COMMAND rxNoise
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