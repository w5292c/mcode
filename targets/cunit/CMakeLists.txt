cmake_minimum_required (VERSION 2.8)

#FIND_PACKAGE(cunit REQUIRED)

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

INCLUDE_DIRECTORIES(
  ${MCODE_TOP}/src/
#  ${PROJECT_BINARY_DIR}/include/
)

#set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage" )

#set ( UDMD_TOP "${CMAKE_SOURCE_DIR}/udmd/" )

set ( CUNIT_SRC_LIST
  ${MCODE_TOP}/src/tests/cunit-main.c
  ${MCODE_TOP}/src/common/mparser.c
)

add_executable ( mcode-cunit ${CUNIT_SRC_LIST} )

target_link_libraries ( mcode-cunit -lcunit )

add_test ( mcode-cunit mcode-cunit )
