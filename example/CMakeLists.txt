if (NOT MOCK_SOURCES)

add_executable(brakelight.elf ${CMAKE_CURRENT_SOURCE_DIR}/brakelight/main.cpp)
target_link_libraries(brakelight.elf PRIVATE MercuryCAN)
target_include_directories(brakelight.elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(relays.elf ${CMAKE_CURRENT_SOURCE_DIR}/relays/relays.cpp)
add_executable(sender.elf ${CMAKE_CURRENT_SOURCE_DIR}/relays/sender.cpp)
target_link_libraries(relays.elf PRIVATE MercuryCAN)
target_link_libraries(sender.elf PRIVATE MercuryCAN)
target_include_directories(relays.elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(sender.elf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
embed_dbc(relays.elf ${CMAKE_CURRENT_SOURCE_DIR}/relays/database.dbc DBC)
embed_dbc(sender.elf ${CMAKE_CURRENT_SOURCE_DIR}/relays/database.dbc DBC)

endif()