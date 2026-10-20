function(embed_dbc target dbc_file variable_name)
    if(NOT EXISTS "${dbc_file}")
        message(FATAL_ERROR "DBC file does not exist: ${dbc_file}")
    endif()
    if(NOT variable_name MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR "Invalid C++ variable name: ${variable_name}")
    endif()

    set(generated_source "${CMAKE_CURRENT_BINARY_DIR}/${variable_name}.cpp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                 "${dbc_file}")

    file(READ "${dbc_file}" database_hex HEX)
    string(LENGTH "${database_hex}" database_hex_length)
    set(database_initializer "")

    if(database_hex_length GREATER 0)
        math(EXPR database_last_byte "${database_hex_length} - 2")
        set(database_byte_count 0)

        foreach(database_index RANGE 0 ${database_last_byte} 2)
            string(SUBSTRING "${database_hex}" ${database_index} 2
                   database_byte)
            string(APPEND database_initializer "0x${database_byte}, ")
            math(EXPR database_byte_count "${database_byte_count} + 1")
            math(EXPR database_line_position "${database_byte_count} % 12")
            if(database_line_position EQUAL 0)
                string(APPEND database_initializer "\n    ")
            endif()
        endforeach()
    endif()

    file(WRITE "${generated_source}"
         "extern const char ${variable_name}[] = {\n    ${database_initializer}0x00\n};\n")
    target_sources(${target} PRIVATE "${generated_source}")
endfunction()
