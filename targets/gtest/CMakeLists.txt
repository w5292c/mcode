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

cmake_minimum_required(VERSION 2.6)

include ( ${CMAKE_CURRENT_SOURCE_DIR}/../common/mcode-common.cmake )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

option ( MCODE_PDU "Enable PDU support" ON )
option ( MCODE_COVERAGE "Enable code coverage" ON )
option ( MCODE_UART2 "Enable UART2 module in SoC" ON )
option ( MCODE_NEW_ENGINE "Enable new Command Engine" ON )

# Locate GTest
find_package ( GTest REQUIRED )
find_package ( PkgConfig REQUIRED )

include_directories(
  ${GTEST_INCLUDE_DIRS}
  ${MCODE_TOP}/src
  ${MCODE_TOP}/src/common
  ${PROJECT_BINARY_DIR}/include
)

set (
  SRC_LIST
  # Source code files
  ${MCODE_TOP}/src/common/mvars.c
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/mtimer.c
  ${MCODE_TOP}/src/common/mparser.c
  ${MCODE_TOP}/src/common/mstatus.c
  ${MCODE_TOP}/src/common/mstring.c
  ${MCODE_TOP}/src/common/hw-uart.c
  ${MCODE_TOP}/src/common/cmd-engine.c
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
  ${MCODE_TOP}/src/common/cmd-help.c
  ${MCODE_TOP}/src/gtest/wrap-mocks.cpp
  ${MCODE_TOP}/src/gtest/gtest-main.cpp
  ${MCODE_TOP}/src/emu/persistent-store.c
  ${MCODE_TOP}/src/gtest/test-mtimer.cpp
  ${MCODE_TOP}/src/gtest/test-hw-uart.cpp
  ${MCODE_TOP}/src/gtest/test-scheduler.cpp
  ${MCODE_TOP}/src/gtest/test-mvars-basic.cpp
  ${MCODE_TOP}/src/gtest/test-utils-basic.cpp
  ${MCODE_TOP}/src/gtest/test-mparser-basic.cpp
  ${MCODE_TOP}/src/gtest/test-strings-basic.cpp
  ${MCODE_TOP}/src/gtest/test-strings-mocked.cpp
  ${MCODE_TOP}/src/gtest/test-gsm-engine-basic.cpp
)

publish_phone ()
enable_git_version ( ON )
enable_random_numbers ( 16 )

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
  ${GTEST_LIBRARIES}
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
