cmake_minimum_required (VERSION 2.6)

project (console-test C)

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

set ( SRC_LIST
  ${MCODE_TOP}/src/main-x86.c
  ${MCODE_TOP}/src/hw-i80.c
  ${MCODE_TOP}/src/hw-uart.c
  ${MCODE_TOP}/src/cmd-engine.c
  ${MCODE_TOP}/src/line-editor-uart.c
)

option ( MCODE_EMULATE_UART "Enable UART emulation" ON )
option ( MCODE_EMULATE_I80 "Enable I80 interface emulation" ON )
option ( MCODE_EMULATE_I80_LCD1 "Add LCD1 device to I80 bus" ON )
option ( USE_EMULATION_LAYER "Build Library with emulator layer" ON )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

if (MCODE_EMULATE_UART)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-uart.c )
endif()
if (MCODE_EMULATE_I80)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-i80.c )
endif ()
if (MCODE_EMULATE_I80_LCD1)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-i80-lcd1.c )
endif ()

add_executable( console-test ${SRC_LIST} )

find_package (Threads)
target_link_libraries (console-test ${CMAKE_THREAD_LIBS_INIT})

include_directories ( "${PROJECT_BINARY_DIR}/include/" )

if (MCODE_EMULATE_UART)
  message ( STATUS "CONFIG: Enabled UART emulation" )
endif ()

if (MCODE_EMULATE_I80)
  message ( STATUS "CONFIG: Enabled I80 emulation" )
endif ()

if (MCODE_EMULATE_I80_LCD1)
  message ( STATUS "CONFIG: Emulated LCD1 device is connected to I80 bus" )
endif ()
