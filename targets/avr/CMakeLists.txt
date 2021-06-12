# The MIT License (MIT)
#
# Copyright (c) 2014-2020 Alexander Chumakov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required (VERSION 2.6)

add_definitions ( -mmcu=atmega32 -g -Os -Wall -Wstrict-prototypes )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

include_directories( ${MCODE_TOP}/src/ ${MCODE_TOP}/src/security/ )

set ( CMAKE_SYSTEM_NAME "Generic" )
set ( CMAKE_C_COMPILER "avr-gcc" )
set ( CMAKE_CXX_COMPILER "" )
set ( CMAKE_SYSTEM_PROCESSOR "atmega32" )
set ( CMAKE_FIND_ROOT_PATH "/usr/lib/avr/" )

project (console-test C)

set (SRC_LIST
  ${MCODE_TOP}/src/avr/main.c
  ${MCODE_TOP}/src/common/mvars.c
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/hw-lcd.c
  ${MCODE_TOP}/src/common/mtimer.c
  ${MCODE_TOP}/src/common/hw-uart.c
  ${MCODE_TOP}/src/common/console.c
  ${MCODE_TOP}/src/common/mparser.c
  ${MCODE_TOP}/src/common/mstatus.c
  ${MCODE_TOP}/src/common/mstring.c
  ${MCODE_TOP}/src/common/scheduler.c
  ${MCODE_TOP}/src/common/cmd-help.c
  ${MCODE_TOP}/src/common/cmd-engine.c
  ${MCODE_TOP}/src/common/cmd-system.c
  ${MCODE_TOP}/src/common/cmd-test-image.c
  ${MCODE_TOP}/src/common/line-editor-uart.c
  ${MCODE_TOP}/src/fonts.c
  ${MCODE_TOP}/src/avr/mtick.c
  ${MCODE_TOP}/src/avr/hw-nvm.c
  ${MCODE_TOP}/src/avr/system.c
  ${MCODE_TOP}/src/avr/hw-uart.c
  ${MCODE_TOP}/src/avr/cmd-system.c
)

option ( MCODE_TV "Enable TV control" ON )
option ( MCODE_IR "Enable IR support" OFF )
option ( MCODE_PWM "Enable PWM support" ON )
option ( MCODE_LCD "Enable LCD support" ON )
option ( MCODE_TWI "Enable TWI support" ON )
option ( MCODE_RTC "Enable RTC support" ON )
option ( MCODE_WDT "Enable WDT support" ON )
option ( MCODE_LEDS "Enable LEDs support" OFF )
option ( MCODE_SOUND "Enable sound support" OFF )
option ( MCODE_TUNE_TRACK "Show notes for tunes" OFF )
option ( MCODE_CONSOLE "Enable console support" ON )
option ( MCODE_SECURITY "Enable security support" ON )
option ( MCODE_TEST_STRINGS "Enable test strings" OFF )
option ( MCODE_AVR_FUSE "Enable FUSE configuration" OFF )
option ( MCODE_NEW_ENGINE "Enable new Command Engine" ON )
option ( MCODE_COMMAND_MODES "Enable command engine modes" ON )
option ( MCODE_TEST_IMAGES "Enable hard-coded test images" ON )
option ( MCODE_HW_I80_ENABLED "HW I80 interface is enabled" OFF )
option ( MCODE_CONSOLE_ENABLED "Concole implementation exists" ON )

option ( MCODE_RTC_DS3231 "Enable DS3231 RTC" ON )

option ( MCODE_LCD_ILI9341 "Configure the lib for ILI9341 LCD" ON )
option ( MCODE_LCD_ILI9481 "Configure the lib for ILI9481 LCD" OFF )

option ( MCODE_PERSIST_STORE "Enable persistent store" ON )
option ( MCODE_PERSIST_STORE_EXT_RAM "Enable persistent store on external NVM-RAM" OFF )
option ( MCODE_PERSIST_STORE_EXT_EEPROM "Enable persistent store on external EEPROM" ON )

option ( MCODE_BOOTLOADER "Include AVR109-based bootloader code" ON )

if ( MCODE_LCD )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-lcd.c
  )
endif ( MCODE_LCD )

if ( MCODE_LCD_ILI9341 )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-spi.c
    ${MCODE_TOP}/src/avr/hw-lcd.c
    ${MCODE_TOP}/src/avr/hw-lcd-spi.c
    ${MCODE_TOP}/src/common/hw-lcd-spi.c
    ${MCODE_TOP}/src/common/hw-lcd-ili9341.c
  )
endif ( MCODE_LCD_ILI9341 )

if ( MCODE_LCD_ILI9481 )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-lcd.c
    ${MCODE_TOP}/src/avr/hw-i80.c
    ${MCODE_TOP}/src/avr/hw-lcd-i80.c
    ${MCODE_TOP}/src/common/hw-lcd-i80.c
    ${MCODE_TOP}/src/common/hw-lcd-ili9481.c
  )
endif ( MCODE_LCD_ILI9481 )

if ( MCODE_TWI )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-twi.c
    ${MCODE_TOP}/src/common/cmd-twi.c
  )
endif ( MCODE_TWI )

if ( MCODE_AVR_FUSES )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/fuses.c
  )
endif ( MCODE_AVR_FUSES )

if ( MCODE_LEDS )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-leds.c
    ${MCODE_TOP}/src/common/cmd-leds.c
  )
endif ( MCODE_LEDS )

if ( MCODE_PWM )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-pwm.c
    ${MCODE_TOP}/src/common/cmd-pwm.c
  )
endif ( MCODE_PWM )

if ( MCODE_SECURITY )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-ssl.c
    ${MCODE_TOP}/src/security/librock_sha256.c
  )
endif ( MCODE_SECURITY )

if ( MCODE_PERSIST_STORE )
  if ( MCODE_PERSIST_STORE_EXT_EEPROM )
    set ( SRC_LIST ${SRC_LIST}
      ${MCODE_TOP}/src/common/persistent-store-ext-eeprom.c
    )
  elseif ( MCODE_PERSIST_STORE_EXT_RAM )
    set ( SRC_LIST ${SRC_LIST}
      ${MCODE_TOP}/src/common/persistent-store-ext-ram.c
    )
  else ( MCODE_PERSIST_STORE_EXT_EEPROM )
    set ( SRC_LIST ${SRC_LIST}
      ${MCODE_TOP}/src/avr/persistent-store.c
    )
  endif ( MCODE_PERSIST_STORE_EXT_EEPROM )
endif ( MCODE_PERSIST_STORE )

if ( MCODE_RTC )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/hw-rtc.c
    ${MCODE_TOP}/src/common/cmd-rtc.c
  )
endif ( MCODE_RTC )

if ( MCODE_WDT )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-wdt.c
  )
endif ( MCODE_WDT )

if ( MCODE_RTC_DS3231 )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-rtc-ds3231.c
    ${MCODE_TOP}/src/common/hw-rtc-ds3231.c
  )
endif ( MCODE_RTC_DS3231 )

if ( MCODE_TV )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/cmd-tv.c
    ${MCODE_TOP}/src/common/cmd-tv.c
  )
endif ( MCODE_TV )

if ( MCODE_CONSOLE_ENABLED )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-console.c
  )
endif ( MCODE_CONSOLE_ENABLED )

if ( MCODE_SOUND )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-sound.c
    ${MCODE_TOP}/src/common/cmd-sound.c
  )
endif ( MCODE_SOUND )

if ( MCODE_BOOTLOADER )
  include ( "bootloader.cmake" )
endif ( MCODE_BOOTLOADER )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

add_executable( console-test ${SRC_LIST} )
target_link_libraries (console-test "-mmcu=atmega32")

include_directories ( "${PROJECT_BINARY_DIR}/include/" )

add_custom_command (
  TARGET console-test
  POST_BUILD
  COMMAND avr-size console-test
  COMMAND avr-objdump -h -S console-test > console-test.lst
  COMMAND avr-objcopy -j .text -j command_section -j .data -O ihex console-test console-test.flash.hex
  COMMAND avr-objcopy --change-section-lma .eeprom=0 -O ihex -j .eeprom console-test console-test.eeprom.hex
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Building HEX files to flash to CPU"
)

#[ALL]
add_custom_target(flash
  COMMAND avrdude -b 115200 -c avr109 -p m32 -P /dev/ttyUSB0 -U flash:w:console-test.flash.hex -U eeprom:w:console-test.eeprom.hex
  DEPENDS console-test ${PROJECT_BINARY_DIR}/console-test.flash.hex ${PROJECT_BINARY_DIR}/console-test.eeprom.hex
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Flashing..."
)

add_custom_target ( sizes
  COMMAND avr-nm --print-size --size-sort --radix=d ${PROJECT_BINARY_DIR}/console-test
  DEPENDS console-test
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Show detailed information about sizes in the executable per simbols"
)
