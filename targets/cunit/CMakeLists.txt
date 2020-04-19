cmake_minimum_required (VERSION 2.8)

include ( ${CMAKE_CURRENT_SOURCE_DIR}/../common/mcode-common.cmake )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

INCLUDE_DIRECTORIES(
  ${MCODE_TOP}/src/
  ${MCODE_TOP}/src/common/
  ${MCODE_TOP}/src/security/
  ${PROJECT_BINARY_DIR}/include/
)

set ( CUNIT_SRC_LIST
  ${MCODE_TOP}/src/tests/cunit-main.c
  ${MCODE_TOP}/src/emu/hw-nvm.c
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/mvars.c
  ${MCODE_TOP}/src/common/hw-rtc.c
  ${MCODE_TOP}/src/common/mparser.c
  ${MCODE_TOP}/src/common/mstatus.c
  ${MCODE_TOP}/src/common/mstring.c
  ${MCODE_TOP}/src/emu/persistent-store.c
)

option ( MCODE_UART2 "Enable UART2" ON )
option ( MCODE_PDU "Enable PDU support" ON )
option ( MCODE_RTC "Enable RTC support" ON )
option ( MCODE_NEW_ENGINE "Enable new Command Engine" ON )
option ( MCODE_OLD_PARSER "Enable the obsolete parser" ON )

enable_git_version ( 1 )
enable_random_numbers ( 16 )

configure_file ( ${MCODE_TOP}/mcode-config.h.in
  ${PROJECT_BINARY_DIR}/include/mcode-config.h
)

add_executable ( mcode-cunit ${CUNIT_SRC_LIST} )

target_link_libraries ( mcode-cunit -lcunit )

add_test ( mcode-cunit mcode-cunit )
