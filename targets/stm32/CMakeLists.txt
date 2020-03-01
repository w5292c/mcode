cmake_minimum_required (VERSION 2.8)

project (console-test)
ENABLE_LANGUAGE(ASM)

FIND_PACKAGE(CMSIS REQUIRED)
FIND_PACKAGE(StdPeriphLib REQUIRED)

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

INCLUDE_DIRECTORIES(
    ${MCODE_TOP}/src/
    ${MCODE_TOP}/src/stm32/
    ${MCODE_TOP}/src/security/
    ${PROJECT_BINARY_DIR}/include/
    ${CMSIS_INCLUDE_DIR}
    ${StdPeriphLib_INCLUDE_DIR}
)

set ( SRC_LIST
  ${MCODE_TOP}/src/stm32/main.c
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/hw-lcd.c
  ${MCODE_TOP}/src/common/console.c
  ${MCODE_TOP}/src/common/hw-uart.c
  ${MCODE_TOP}/src/common/mstring.c
  ${MCODE_TOP}/src/common/cmd-help.c
  ${MCODE_TOP}/src/common/scheduler.c
  ${MCODE_TOP}/src/common/hw-lcd-spi.c
  ${MCODE_TOP}/src/common/cmd-system.c
  ${MCODE_TOP}/src/common/cmd-engine.c
  ${MCODE_TOP}/src/common/hw-lcd-ili9341.c
  ${MCODE_TOP}/src/common/cmd-test-image.c
  ${MCODE_TOP}/src/common/line-editor-uart.c
  ${MCODE_TOP}/src/fonts.c
  ${MCODE_TOP}/src/stm32/mtick.c
  ${MCODE_TOP}/src/stm32/system.c
  ${MCODE_TOP}/src/stm32/hw-spi.c
  ${MCODE_TOP}/src/stm32/hw-leds.c
  ${MCODE_TOP}/src/stm32/hw-uart.c
  ${MCODE_TOP}/src/stm32/hw-lcd-spi.c
)

set ( STM32_LINKER_SCRIPT ${CMSIS_LINKER_SCRIPT} )

option ( MCODE_GSM "Enable GSM engine" ON )
option ( MCODE_LCD "Enable LCD support" ON )
option ( MCODE_RTC "Enable RTC support" ON )
option ( MCODE_PARSER "Enable string parser" ON )
option ( MCODE_PROG "Enable programming commands" ON )
option ( MCODE_SECURITY "Enable security support" ON )
option ( MCODE_UART2 "Enable UART2 module in SoC" ON )
option ( MCODE_SWITCH_ENGINE "Enable the Switch Engine" ON )
option ( MCODE_TEST_IMAGES "Enable hard-coded test images" ON )
option ( MCODE_DEBUG_BLINKING "Enable debug LEDs blinking" OFF )
option ( MCODE_HW_I80_ENABLED "HW I80 interface is enabled" OFF )
option ( MCODE_CONSOLE_ENABLED "Concole implementation exists" ON )

if ( MCODE_LCD )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-lcd.c
  )
endif ( MCODE_LCD )

if ( MCODE_CONSOLE_ENABLED )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-console.c
  )
endif ( MCODE_CONSOLE_ENABLED )

if ( MCODE_GSM )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-gsm.c
    ${MCODE_TOP}/src/common/gsm-engine-uart2.c
    ${MCODE_TOP}/src/stm32/gsm-engine-uart2.c
  )
endif ( MCODE_GSM )

if ( MCODE_PARSER )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/mparser.c
  )
endif ( MCODE_PARSER )

if ( MCODE_SWITCH_ENGINE )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-switch.c
    ${MCODE_TOP}/src/stm32/switch-engine.c
  )
endif ( MCODE_SWITCH_ENGINE )

if ( MCODE_SECURITY )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-ssl.c
    ${MCODE_TOP}/src/security/librock_sha256.c
  )
endif ( MCODE_SECURITY )

if ( MCODE_RTC )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/stm32/hw-rtc.c
    ${MCODE_TOP}/src/common/hw-rtc.c
    ${MCODE_TOP}/src/common/cmd-rtc.c
  )
endif ( MCODE_RTC )

if ( MCODE_PROG )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/mvars.c
    ${MCODE_TOP}/src/common/cmd-prog.c
    ${MCODE_TOP}/src/stm32/hw-nvm.c
  )
endif ( MCODE_PROG )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

add_executable(${CMAKE_PROJECT_NAME} ${SRC_LIST} ${CMSIS_STARTUP_SOURCE} ${CMSIS_SOURCES} ${StdPeriphLib_SOURCES})

# Add a Linker Script for including custom sections
target_link_libraries ( ${CMAKE_PROJECT_NAME}
  -T${MCODE_TOP}/targets/stm32/console-test_custom.ld
)

STM32_SET_TARGET_PROPERTIES(${CMAKE_PROJECT_NAME})
STM32_SET_HSE_VALUE(${CMAKE_PROJECT_NAME} 8000000)
STM32_ADD_HEX_BIN_TARGETS(${CMAKE_PROJECT_NAME})

add_custom_command (
  TARGET console-test
  POST_BUILD
  COMMAND arm-none-eabi-size console-test
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Show some statistics"
)
