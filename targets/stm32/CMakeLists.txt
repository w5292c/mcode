cmake_minimum_required (VERSION 2.8)

project (console-test)
ENABLE_LANGUAGE(ASM)

FIND_PACKAGE(CMSIS REQUIRED)
FIND_PACKAGE(StdPeriphLib REQUIRED)

add_definitions( "-D__ARM__ -D__HHCSTM32F100__" )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

INCLUDE_DIRECTORIES(
    ${MCODE_TOP}/src/
    ${MCODE_TOP}/src/stm32/
    ${PROJECT_BINARY_DIR}/include/
    ${CMSIS_INCLUDE_DIR}
    ${StdPeriphLib_INCLUDE_DIR}
)

set ( SRC_LIST
  ${MCODE_TOP}/src/stm32/main.c
  ${MCODE_TOP}/src/common/hw-lcd.c
  ${MCODE_TOP}/src/utils.c
  ${MCODE_TOP}/src/fonts.c
  ${MCODE_TOP}/src/console.c
  ${MCODE_TOP}/src/scheduler.c
  ${MCODE_TOP}/src/cmd-engine.c
  ${MCODE_TOP}/src/stm32/mtick.c
  ${MCODE_TOP}/src/stm32/hw-leds.c
  ${MCODE_TOP}/src/stm32/hw-uart.c
#  ${MCODE_TOP}/src/cmd-test-image.c
  ${MCODE_TOP}/src/line-editor-uart.c
  ${MCODE_TOP}/src/stm32/hw-lcd-spi.c
)

set ( STM32_LINKER_SCRIPT ${CMSIS_LINKER_SCRIPT} )

option ( MCODE_DEBUG_BLINKING "Enable debug LEDs blinking" OFF )
option ( MCODE_HW_I80_ENABLED "HW I80 interface is enabled" OFF )
option ( MCODE_HW_UART_SYNC_WRITE "Use synchronous UART write" ON )
option ( MCODE_CONSOLE_ENABLED "Concole implementation exists" ON )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

add_executable(${CMAKE_PROJECT_NAME} ${SRC_LIST} ${CMSIS_STARTUP_SOURCE} ${CMSIS_SOURCES} ${StdPeriphLib_SOURCES})

STM32_SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME})
STM32_SET_HSE_VALUE(${CMAKE_PROJECT_NAME} 8000000)
STM32_ADD_HEX_BIN_TARGETS(${CMAKE_PROJECT_NAME})
