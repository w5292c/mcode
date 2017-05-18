cmake_minimum_required (VERSION 2.8)

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

INCLUDE_DIRECTORIES(
  ${MCODE_TOP}/src/
  ${PROJECT_BINARY_DIR}/include/
)

set ( CUNIT_SRC_LIST
  ${MCODE_TOP}/src/tests/cunit-main.c
  ${MCODE_TOP}/src/common/hw-rtc.c
  ${MCODE_TOP}/src/common/mparser.c
)

option ( MCODE_RTC "Enable RTC support" ON )

configure_file ( ${MCODE_TOP}/mcode-config.h.in
  ${PROJECT_BINARY_DIR}/include/mcode-config.h
)

add_executable ( mcode-cunit ${CUNIT_SRC_LIST} )

target_link_libraries ( mcode-cunit -lcunit )

add_test ( mcode-cunit mcode-cunit )
