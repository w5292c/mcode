cmake_minimum_required (VERSION 2.6)

add_definitions( -mmcu=atmega32 -g -Os -Wall -Wstrict-prototypes )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )
set ( MCODE_SSL ${MCODE_TOP}/src/openssl/ )

include_directories( ${MCODE_TOP}/src/ ${MCODE_SSL} )

set ( CMAKE_SYSTEM_NAME "Generic" )
set ( CMAKE_C_COMPILER "avr-gcc" )
set ( CMAKE_CXX_COMPILER "" )
set ( CMAKE_SYSTEM_PROCESSOR "atmega32" )
set ( CMAKE_FIND_ROOT_PATH "/usr/lib/avr/" )

project (console-test C)

set (SRC_LIST
  ${MCODE_TOP}/src/avr/main.c
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/hw-uart.c
  ${MCODE_TOP}/src/common/mstring.c
  ${MCODE_TOP}/src/common/scheduler.c
  ${MCODE_TOP}/src/common/cmd-engine.c
  ${MCODE_TOP}/src/common/cmd-system.c
  ${MCODE_TOP}/src/common/line-editor-uart.c
  ${MCODE_TOP}/src/avr/mtick.c
  ${MCODE_TOP}/src/avr/system.c
  ${MCODE_TOP}/src/avr/hw-uart.c
)

option ( MCODE_TV "Enable TV control" ON )
option ( MCODE_TWI "Enable TWI support" ON )
option ( MCODE_RTC "Enable RTC support" ON )
option ( MCODE_PWM "Enable PWM support" OFF )
option ( MCODE_LCD "Enable LCD support" OFF )
option ( MCODE_LEDS "Enable LEDs support" OFF )
option ( MCODE_SOUND "Enable sound support" ON )
option ( MCODE_TUNE_TRACK "Show notes for tunes" OFF )
option ( MCODE_CONSOLE "Enable console support" OFF )
option ( MCODE_SECURITY "Enable security support" ON )
option ( MCODE_AVR_FUSE "Enable FUSE configuration" OFF )
option ( MCODE_COMMAND_MODES "Enable command engine modes" ON )
option ( MCODE_TEST_IMAGES "Enable hard-coded test images" OFF )
option ( MCODE_HW_I80_ENABLED "HW I80 interface is enabled" OFF )
option ( MCODE_CONSOLE_ENABLED "Concole implementation exists" OFF )

option ( MCODE_RTC_DS3231 "Enable DS3231 RTC" ON )

option ( MCODE_LCD_ILI9341 "Configure the lib for ILI9341 LCD" OFF )
option ( MCODE_LCD_ILI9481 "Configure the lib for ILI9481 LCD" OFF )

option ( MCODE_PERSIST_STORE "Enable persistent store" ON )
option ( MCODE_PERSIST_STORE_EXT_RAM "Enable persistent store on external NVM-RAM" OFF )
option ( MCODE_PERSIST_STORE_EXT_EEPROM "Enable persistent store on external EEPROM" ON )

option ( MCODE_BOOTLOADER "Include AVR109-based bootloader code" ON )

if ( MCODE_LCD )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/fonts.c
    ${MCODE_TOP}/src/common/hw-lcd.c
    ${MCODE_TOP}/src/common/cmd-lcd.c
  )
endif ( MCODE_LCD )

if ( MCODE_TWI )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-twi.c
    ${MCODE_TOP}/src/common/cmd-twi.c
  )
endif ( MCODE_TWI )

if ( MCODE_CONSOLE )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/console.c
  )
endif ( MCODE_CONSOLE )

if ( MCODE_TEST_IMAGES )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-test-image.c
  )
endif ( MCODE_TEST_IMAGES )

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
    ${MCODE_TOP}/src/common/cmd-lcd.c
    ${MCODE_TOP}/src/common/hw-lcd-i80.c
    ${MCODE_TOP}/src/common/hw-lcd-ili9481.c
  )
endif ( MCODE_LCD_ILI9481 )

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
  add_definitions ( -DMD32_REG_T=long )

  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_SSL}/sha256.c
    ${MCODE_SSL}/mem_clr.c
    ${MCODE_TOP}/src/common/cmd-ssl.c
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

if ( MCODE_SOUND )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/avr/hw-sound.c
    ${MCODE_TOP}/src/common/cmd-sound.c
  )
endif ( MCODE_SOUND )

if ( MCODE_BOOTLOADER )
  include ( "../avr/bootloader.cmake" )
endif ( MCODE_BOOTLOADER )

include_directories ( "${PROJECT_BINARY_DIR}/include/" )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

add_executable( console-test ${SRC_LIST} )
target_link_libraries (console-test "-mmcu=atmega32")

add_custom_command (
  TARGET console-test
  POST_BUILD
  COMMAND avr-size console-test
  COMMAND avr-objdump -h -S console-test > console-test.lst
  COMMAND avr-objcopy -j .text -j .data -O ihex console-test console-test.flash.hex
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
