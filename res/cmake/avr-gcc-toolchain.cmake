set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

set(MCU at90can128 CACHE STRING "AVR MCU")
set(FRQ 16000000UL CACHE STRING "AVR CPU frequency")

set(CMAKE_C_FLAGS
    "-mmcu=${MCU} -DF_CPU=${FRQ} -Ofast -Wall"
    CACHE STRING "AVR compiler flags"
)

set(CMAKE_CXX_FLAGS
    "-mmcu=${MCU} -DF_CPU=${FRQ} -Ofast -Wall -fno-threadsafe-statics"
    CACHE STRING "AVR compiler flags"
)
set(CMAKE_EXE_LINKER_FLAGS
    "-mmcu=${MCU}"
    CACHE STRING "AVR linker flags"
)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)