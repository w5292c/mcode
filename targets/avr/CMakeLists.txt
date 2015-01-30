cmake_minimum_required (VERSION 2.6)

add_definitions( "-mmcu=atmega32 -g -Os -Wall -Wstrict-prototypes" )
set ( CMAKE_EXE_LINKER_FLAGS "-mmcu=atmega32" )
set ( CMAKE_SHARED_LINKER_FLAGS "-mmcu=atmega32" )
set ( CMAKE_MODULE_LINKER_FLAGS "-mmcu=atmega32" )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )
include_directories( ${MCODE_TOP}/src/ )

set ( CMAKE_SYSTEM_NAME "Generic" )
set ( CMAKE_C_COMPILER "avr-gcc" )
set ( CMAKE_CXX_COMPILER "" )
set ( CMAKE_SYSTEM_PROCESSOR "atmega32" )
set ( CMAKE_FIND_ROOT_PATH "/usr/lib/avr/" )

project (console-test C)

set (SRC_LIST
  ${MCODE_TOP}/src/avr/main.c
  ${MCODE_TOP}/src/common/hw-lcd.c
  ${MCODE_TOP}/src/fonts.c
  ${MCODE_TOP}/src/utils.c
  ${MCODE_TOP}/src/console.c
  ${MCODE_TOP}/src/hw-uart.c
  ${MCODE_TOP}/src/scheduler.c
  ${MCODE_TOP}/src/avr/mtick.c
#  ${MCODE_TOP}/src/avr/fuses.c
  ${MCODE_TOP}/src/avr/hw-i80.c
  ${MCODE_TOP}/src/cmd-engine.c
  ${MCODE_TOP}/src/avr/hw-lcd.c
  ${MCODE_TOP}/src/avr/hw-leds.c
  ${MCODE_TOP}/src//cmd-test-image.c
  ${MCODE_TOP}/src/line-editor-uart.c
  ${MCODE_TOP}/src/avr/hw-lcd-s95513.c
)

option ( MCODE_EMULATE_UART "Enable UART emulation" OFF )
option ( MCODE_EMULATE_I80 "Enable I80 interface emulation" OFF )
option ( MCODE_EMULATE_I80_LCD1 "Add LCD1 device to I80 bus" OFF )
option ( MCODE_HW_UART_SYNC_WRITE "Use synchronous UART write" ON )
option ( USE_EMULATION_LAYER "Build Library with emulator layer" OFF )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

if (MCODE_EMULATE_UART)
  set ( SRC_LIST ${SRC_LIST} src/emu-hw-uart.c )
  find_package (Threads)
  target_link_libraries (console-test ${CMAKE_THREAD_LIBS_INIT})
endif()
if (MCODE_EMULATE_I80)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-i80.c )
endif ()
if (MCODE_EMULATE_I80_LCD1)
  set ( SRC_LIST ${SRC_LIST} ${MCODE_TOP}/src/emu-hw-i80-lcd1.c )
endif ()

add_executable( console-test ${SRC_LIST} )
target_link_libraries (console-test "-mmcu=atmega32")

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

add_custom_command (
  TARGET console-test
  POST_BUILD
  COMMAND avr-objcopy -O ihex -j .text -j .bss -j .data console-test console-test.flash.hex
  COMMAND avr-objcopy -O ihex -j .eeprom console-test console-test.eeprom.hex
  COMMAND avr-objcopy -O ihex -j .fuse console-test console-test.fuse.hex
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Building HEX files to flash to CPU"
)

#[ALL]
add_custom_target(flash
  COMMAND sudo avrdude -B 8 -p m32 -c jtagmkI -P /dev/ttyUSB0 -U flash:w:console-test.flash.hex
  DEPENDS console-test ${PROJECT_BINARY_DIR}/console-test.flash.hex
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Flashing..."
)
