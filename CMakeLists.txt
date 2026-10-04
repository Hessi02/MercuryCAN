cmake_minimum_required(VERSION 3.10)

project(MercuryCAN)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

target_include_directories(MercuryCAN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)