cmake_minimum_required (VERSION 2.6)

project (console-test)

set ( STM32_LIBRARY_PATH "${STM32_ROOT}/Libraries/STM32F10x_StdPeriph_Driver/" )

if (DEFINED STM32_ROOT)
  if (EXISTS "${STM32_ROOT}" AND EXISTS "${STM32_LIBRARY_PATH}/src/" AND EXISTS "${STM32_LIBRARY_PATH}//inc/" )
    message( "-- Detected STM32 Standard Peripherals Library: ${STM32_ROOT}" )
  else (EXISTS "${STM32_ROOT}" AND EXISTS "${STM32_LIBRARY_PATH}/src/" AND EXISTS "${STM32_LIBRARY_PATH}//inc/" )
    message( FATAL_ERROR "Fatal: STM32 Standard Peripherals Library is not found at: ${STM32_ROOT}, update STM32_ROOT to point to the right folder" )
  endif (EXISTS "${STM32_ROOT}" AND EXISTS "${STM32_LIBRARY_PATH}/src/" AND EXISTS "${STM32_LIBRARY_PATH}//inc/" )
else (DEFINED STM32_ROOT)
  message( FATAL_ERROR "Fatal: STM32_ROOT is not provided" )
endif (DEFINED STM32_ROOT)

set ( STM32_LINKER_SCRIPT "${STM32_ROOT}/Project/STM32F10x_StdPeriph_Template/TrueSTUDIO/STM3210E-EVAL/stm32_flash.ld" )

if ( EXISTS "${STM32_LINKER_SCRIPT}" )
  message ( "-- Detected linker script" )
else ( EXISTS "${STM32_LINKER_SCRIPT}" )
  message( FATAL_ERROR "Fatal: No linker script" )
endif ( EXISTS "${STM32_LINKER_SCRIPT}" )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

set ( CMAKE_SYSTEM_NAME "Generic" )
set ( CMAKE_C_COMPILER "arm-none-eabi-gcc" )
set ( CMAKE_CXX_COMPILER "" )
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "-lc")
#set(CMAKE_SHARED_LINKER_FLAGS "-lc")
#set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "-lc")
#set ( CMAKE_SYSTEM_PROCESSOR "atmega32" )
#set ( CMAKE_FIND_ROOT_PATH "/usr/lib/avr/" )

include_directories ( ${MCODE_TOP}/src/ )

ADD_DEFINITIONS ( -D__STDC_CONSTANT_MACROS )
ADD_DEFINITIONS ( -D__ARM__ )

set ( SRC_LIST
  ${MCODE_TOP}/src/main-arm.c
  ${MCODE_TOP}/src/scheduler.c
)

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )
add_executable( console-test ${SRC_LIST} )

include_directories ( "${PROJECT_BINARY_DIR}/include/" )
