cmake_minimum_required(VERSION 2.6)

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

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
  # Test source code files
  ${MCODE_TOP}/src/gtest/wrap-mocks.cpp
  ${MCODE_TOP}/src/gtest/gtest-main.cpp
  ${MCODE_TOP}/src/gtest/test-strings.cpp
)

configure_file ( ${MCODE_TOP}/mcode-config.h.in
  ${PROJECT_BINARY_DIR}/include/mcode-config.h
)

add_executable(console-test.test ${SRC_LIST})

target_link_libraries(console-test.test ${GTEST_LIBRARIES} pthread "-Wl,--wrap,uart_write_char")
