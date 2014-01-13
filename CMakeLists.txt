cmake_minimum_required (VERSION 2.6)

project (console-test C)

set (SRC_LIST src/main.c
  src/hw-uart.c
  src/cmd-engine.c
  src/line-editor-uart.c)

option ( MCODE_EMULATE_UART "Enable UART emulation" ON )
option ( MCODE_EMULATE_I80 "Enable I80 interface emulation" ON )
option ( MCODE_EMULATE_I80_LCD1 "Add LCD1 device to I80 bus" ON )
option ( USE_EMULATION_LAYER "Build Library with emulator layer" ON )

configure_file ( ${CMAKE_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/include/config.h )

if (MCODE_EMULATE_UART)
  set ( SRC_LIST ${SRC_LIST} src/emu-hw-uart.c )
endif()

add_executable( console-test ${SRC_LIST} )

find_package (Threads)
target_link_libraries (console-test ${CMAKE_THREAD_LIBS_INIT})

include_directories ( "${PROJECT_BINARY_DIR}/include/" )

if (MCODE_EMULATE_UART)
  message ( STATUS "Enabled UART emulation" )
endif ()

if (MCODE_EMULATE_I80)
  message ( STATUS "Enabled I80 emulation" )
endif ()

if (MCODE_EMULATE_I80_LCD1)
  message ( STATUS "Emulated LCD1 device is connected to I80 bus" )
endif ()
