cmake_minimum_required (VERSION 2.6)

project (console-test)

find_package ( Threads )
find_package ( Qt4       REQUIRED )
find_package ( PkgConfig REQUIRED )

pkg_check_modules ( GTK2_0        gtk+-2.0        REQUIRED )
link_directories ( ${GTK2_0_LIBRARY_DIRS} )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

include_directories ( ${GTK2_0_INCLUDE_DIRS} ${MCODE_TOP}/src/ )

INCLUDE ( ${QT_USE_FILE} )
ADD_DEFINITIONS ( ${QT_DEFINITIONS} )
ADD_DEFINITIONS ( -D__STDC_CONSTANT_MACROS )

set ( SRC_LIST
  ${MCODE_TOP}/src/main-x86.cpp
  ${MCODE_TOP}/src/fonts.c
  ${MCODE_TOP}/src/hw-i80.c
  ${MCODE_TOP}/src/console.c
  ${MCODE_TOP}/src/hw-leds.c
  ${MCODE_TOP}/src/hw-uart.c
  ${MCODE_TOP}/src/scheduler.c
  ${MCODE_TOP}/src/cmd-engine.c
  ${MCODE_TOP}/src/hw-lcd-s95513.c
  ${MCODE_TOP}/src/cmd-test-image.c
  ${MCODE_TOP}/src/line-editor-uart.c
)

set ( SRC_LIST_QT
  ${MCODE_TOP}/src/customwidget.cpp
)

set ( HEADERS_LIST_QT
  ${MCODE_TOP}/src/customwidget.h
)

QT4_WRAP_CPP ( HEADERS_MOC ${HEADERS_LIST_QT} )

option ( MCODE_EMULATE_LED "Enable LEDs emulation" ON )
option ( MCODE_EMULATE_UART "Enable UART emulation" ON )
option ( MCODE_EMULATE_I80 "Enable I80 interface emulation" ON )
option ( MCODE_EMULATE_I80_LCD1 "Add LCD1 device to I80 bus" OFF )
option ( USE_EMULATION_LAYER "Build Library with emulator layer" ON )
option ( MCODE_EMULATE_LCD_S95513 "Emulate LCD with controller s95513" ON )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

if (MCODE_EMULATE_UART)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-uart.c )
endif()
if (MCODE_EMULATE_I80)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-i80.c )
endif ()
if (MCODE_EMULATE_I80_LCD1)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-i80-lcd1.c )
endif (MCODE_EMULATE_I80_LCD1)

if (MCODE_EMULATE_LCD_S95513)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-lcd-s95513.cpp )
endif (MCODE_EMULATE_LCD_S95513)

add_executable( console-test ${SRC_LIST} ${HEADERS_MOC} ${SRC_LIST_QT} )
target_link_libraries ( console-test ${GTK2_0_LIBRARIES} )
target_link_libraries ( console-test ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries ( console-test  ${QT_LIBRARIES} )

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
