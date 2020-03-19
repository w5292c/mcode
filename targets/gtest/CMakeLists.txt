cmake_minimum_required(VERSION 2.6)

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

option ( MCODE_PDU "Enable PDU support" ON )
option ( MCODE_COVERAGE "Enable code coverage" ON )
option ( MCODE_UART2 "Enable UART2 module in SoC" ON )
option ( MCODE_NEW_ENGINE "Enable new Command Engine" ON )

# Locate GTest
find_package ( GTest REQUIRED )
find_package ( Qt4       REQUIRED )
find_package ( PkgConfig REQUIRED )

pkg_check_modules ( GLIB2_0       glib-2.0        REQUIRED )

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${GLIB2_0_INCLUDE_DIRS}
  ${QT4_INCLUDE_DIRS}
  ${MCODE_TOP}/src
  ${MCODE_TOP}/src/common
  ${PROJECT_BINARY_DIR}/include
)

INCLUDE ( ${QT_USE_FILE} )
ADD_DEFINITIONS ( ${QT_DEFINITIONS} )

set (
  SRC_LIST
  # Source code files
  ${MCODE_TOP}/src/common/mvars.c
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/mtimer.c
  ${MCODE_TOP}/src/common/mparser.c
  ${MCODE_TOP}/src/common/mstring.c
  ${MCODE_TOP}/src/common/gsm-engine-uart2.c
  ${MCODE_TOP}/src/common/line-editor-uart.c
)

set_source_files_properties (
  ${MCODE_TOP}/src/common/gsm-engine-uart2.c
  PROPERTIES COMPILE_FLAGS "-Dstatic=\"\""
)

set (
  TEST_SRC_LIST
  # Test source code files
  ${MCODE_TOP}/src/emu/mtick.c
  ${MCODE_TOP}/src/emu/hw-nvm.c
  ${MCODE_TOP}/src/emu/hw-uart.c
  ${MCODE_TOP}/src/emu/scheduler.c
  ${MCODE_TOP}/src/gtest/wrap-mocks.cpp
  ${MCODE_TOP}/src/gtest/gtest-main.cpp
  ${MCODE_TOP}/src/emu/persistent-store.c
  ${MCODE_TOP}/src/gtest/test-mtimer.cpp
  ${MCODE_TOP}/src/gtest/test-scheduler.cpp
  ${MCODE_TOP}/src/gtest/test-mvars-basic.cpp
  ${MCODE_TOP}/src/gtest/test-utils-basic.cpp
  ${MCODE_TOP}/src/gtest/test-mparser-basic.cpp
  ${MCODE_TOP}/src/gtest/test-strings-basic.cpp
  ${MCODE_TOP}/src/gtest/test-strings-mocked.cpp
  ${MCODE_TOP}/src/gtest/test-gsm-engine-basic.cpp
)

configure_file ( ${MCODE_TOP}/mcode-config.h.in
  ${PROJECT_BINARY_DIR}/include/mcode-config.h
)

add_library ( console-test.lib STATIC ${SRC_LIST})
if ( MCODE_COVERAGE )
  target_compile_options ( console-test.lib PRIVATE "-fprofile-arcs" "-ftest-coverage" "-O0" )
  target_link_libraries ( console-test.lib gcov )
endif ( MCODE_COVERAGE )

add_executable(console-test.test ${TEST_SRC_LIST})

target_link_libraries(
  console-test.test
  ${QT_LIBRARIES}
  ${GTEST_LIBRARIES}
  ${GLIB2_0_LIBRARIES}
  gmock pthread console-test.lib
  "-Wl,--wrap,uart_write_char,--wrap,uart2_write_char"
)

add_custom_target (
  cov
  DEPENDS console-test.test
  COMMAND ${PROJECT_BINARY_DIR}/console-test.test
  COMMAND mkdir -p ${PROJECT_BINARY_DIR}/lcov
  COMMAND lcov -c -d ${MCODE_TOP} -o ${PROJECT_BINARY_DIR}/lcov/summary.info
  COMMAND mkdir -p ${PROJECT_BINARY_DIR}/html
  COMMAND genhtml -o ${PROJECT_BINARY_DIR}/html ${PROJECT_BINARY_DIR}/lcov/summary.info
)
