set ( MCODE_BL_TOP ${CMAKE_SOURCE_DIR}/../../src/avr/bootloader/ )

include_directories( ${MCODE_BL_TOP} )

set (BL_SRC_LIST
  ${MCODE_BL_TOP}/main.c
)

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

add_executable( bootloader ${BL_SRC_LIST} )
target_link_libraries ( bootloader "-mmcu=atmega32 -Ttext=0x7000 -nostartfiles -nodefaultlibs" )

add_custom_command (
  TARGET bootloader
  POST_BUILD
  COMMAND avr-size bootloader
  COMMAND avr-objdump -h -S bootloader > bootloader.lst
  COMMAND avr-objcopy -j .text -j .data -O ihex bootloader bootloader.flash.hex
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Building HEX files to flash to CPU"
)

add_custom_target(flash-bl
  COMMAND sudo avrdude -B 8 -p m32 -c jtagmkI -P /dev/ttyUSB0 -U flash:w:bootloader.flash.hex
  DEPENDS bootloader ${PROJECT_BINARY_DIR}/bootloader.flash.hex
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Flashing..."
)
