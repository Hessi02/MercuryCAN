function(embed_dbc target dbc_file macro_name)
    if(NOT EXISTS "${dbc_file}")
        message(FATAL_ERROR "DBC file does not exist: ${dbc_file}")
    endif()
    if(NOT macro_name MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR "Invalid preprocessor macro name: ${macro_name}")
    endif()

    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                 "${dbc_file}")

    file(READ "${dbc_file}" database_hex HEX)
    string(LENGTH "${database_hex}" database_hex_length)
    set(database_definition "${macro_name}=\"")

    if(database_hex_length GREATER 0)
        math(EXPR database_last_byte "${database_hex_length} - 2")

        foreach(database_index RANGE 0 ${database_last_byte} 2)
            string(SUBSTRING "${database_hex}" ${database_index} 2
                   database_byte)
            string(APPEND database_definition "\\x${database_byte}")
            if(database_index LESS database_last_byte)
                string(APPEND database_definition "\"\"")
            endif()
        endforeach()
    endif()

    string(APPEND database_definition "\"")
    target_compile_definitions(${target} PRIVATE "${database_definition}")
endfunction()
