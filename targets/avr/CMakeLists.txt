cmake_minimum_required (VERSION 2.6)

add_definitions( "-mmcu=atmega32" )

set ( CMAKE_SYSTEM_NAME "Generic" )
set ( CMAKE_C_COMPILER "avr-gcc" )
set ( CMAKE_CXX_COMPILER "" )
set ( CMAKE_SYSTEM_PROCESSOR "atmega32" )
set ( CMAKE_FIND_ROOT_PATH "/usr/lib/avr/" )

project (console-test C)

set (SRC_LIST
  src/main-x86.c
  src/hw-i80.c
  src/hw-uart.c
  src/cmd-engine.c
  src/line-editor-uart.c)

option ( MCODE_EMULATE_UART "Enable UART emulation" OFF )
option ( MCODE_EMULATE_I80 "Enable I80 interface emulation" OFF )
option ( MCODE_EMULATE_I80_LCD1 "Add LCD1 device to I80 bus" OFF )
option ( USE_EMULATION_LAYER "Build Library with emulator layer" ON )

configure_file ( ${CMAKE_SOURCE_DIR}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

if (MCODE_EMULATE_UART)
  set ( SRC_LIST ${SRC_LIST} src/emu-hw-uart.c )
  find_package (Threads)
  target_link_libraries (console-test ${CMAKE_THREAD_LIBS_INIT})
endif()
if (MCODE_EMULATE_I80)
  set ( SRC_LIST ${SRC_LIST} src/emu-hw-i80.c )
endif ()
if (MCODE_EMULATE_I80_LCD1)
  set ( SRC_LIST ${SRC_LIST} src/emu-hw-i80-lcd1.c )
endif ()

add_executable( console-test ${SRC_LIST} )

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
