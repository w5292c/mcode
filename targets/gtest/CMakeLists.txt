cmake_minimum_required(VERSION 2.6)

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

option ( MCODE_PDU "Enable PDU support" ON )
option ( MCODE_COVERAGE "Enable code coverage" ON )

# Locate GTest
find_package(GTest REQUIRED)
include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${MCODE_TOP}/src
  ${PROJECT_BINARY_DIR}/include/)

set (
  SRC_LIST
  # Source code files
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/mstring.c
)

set (
  TEST_SRC_LIST
  # Test source code files
  ${MCODE_TOP}/src/gtest/wrap-mocks.cpp
  ${MCODE_TOP}/src/gtest/gtest-main.cpp
  ${MCODE_TOP}/src/gtest/test-utils-basic.cpp
  ${MCODE_TOP}/src/gtest/test-strings-basic.cpp
  ${MCODE_TOP}/src/gtest/test-strings-mocked.cpp
)

configure_file ( ${MCODE_TOP}/mcode-config.h.in
  ${PROJECT_BINARY_DIR}/include/mcode-config.h
)

add_library ( console-test.lib STATIC ${SRC_LIST})
if ( MCODE_COVERAGE )
  target_compile_options ( console-test.lib PRIVATE "-fprofile-arcs" "-ftest-coverage" )
  target_link_libraries ( console-test.lib gcov )
endif ( MCODE_COVERAGE )

add_executable(console-test.test ${TEST_SRC_LIST})

target_link_libraries(console-test.test ${GTEST_LIBRARIES} gmock pthread console-test.lib "-Wl,--wrap,uart_write_char")

add_custom_target (
  cov
  DEPENDS console-test.test
  COMMAND ${PROJECT_BINARY_DIR}/console-test.test
  COMMAND mkdir -p ${PROJECT_BINARY_DIR}/lcov
  COMMAND lcov -c -d ${MCODE_TOP} -o ${PROJECT_BINARY_DIR}/lcov/summary.info
  COMMAND mkdir -p ${PROJECT_BINARY_DIR}/html
  COMMAND genhtml -o ${PROJECT_BINARY_DIR}/html ${PROJECT_BINARY_DIR}/lcov/summary.info
)
