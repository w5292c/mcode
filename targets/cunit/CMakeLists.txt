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
  ${MCODE_TOP}/src/common/utils.c
)

option ( MCODE_GSM "Enable GSM engine" ON )
option ( MCODE_PDU "Enable PDU support" ON )
option ( MCODE_RTC "Enable RTC support" ON )

set ( MCODE_RANDOM_BYTES_COUNT 16 )

IF ( ${MCODE_RANDOM_BYTES_COUNT} EQUAL 0 )
  message ( "-- No build-time random data" )
ELSE ()
  message ( "-- Build-time random data support: ${MCODE_RANDOM_BYTES_COUNT} bytes" )
  set ( MCODE_RANDOM_DATA ON BOOL )
  execute_process (
    COMMAND dd if=/dev/urandom count=1 bs=${MCODE_RANDOM_BYTES_COUNT} status=none
    COMMAND hexdump -v -f ${MCODE_TOP}/hexdump.format
    OUTPUT_VARIABLE MCODE_RANDOM_BYTES
  )
  message ( "-- Random bytes: ${MCODE_RANDOM_BYTES}" )
ENDIF ()

configure_file ( ${MCODE_TOP}/mcode-config.h.in
  ${PROJECT_BINARY_DIR}/include/mcode-config.h
)

IF ( MCODE_GSM )
  SET ( CUNIT_SRC_LIST ${CUNIT_SRC_LIST}
    ${MCODE_TOP}/src/common/gsm-utils.c
  )
ENDIF ( MCODE_GSM )

add_executable ( mcode-cunit ${CUNIT_SRC_LIST} )

target_link_libraries ( mcode-cunit -lcunit )

add_test ( mcode-cunit mcode-cunit )
