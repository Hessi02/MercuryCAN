if (NOT MOCK_SOURCES)

add_executable(brakelight.elf ${CMAKE_CURRENT_SOURCE_DIR}/brakelight/main.cpp)
target_link_libraries(brakelight.elf PRIVATE MercuryCAN)
target_include_directories(brakelight.elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(relays.elf ${CMAKE_CURRENT_SOURCE_DIR}/relays/main.cpp)
target_link_libraries(relays.elf PRIVATE MercuryCAN)
target_include_directories(relays.elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

endif()