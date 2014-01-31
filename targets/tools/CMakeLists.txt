cmake_minimum_required (VERSION 2.6)

find_package(PkgConfig REQUIRED)

project (convert-image CXX)

pkg_check_modules (GIO_UNIX     gio-unix-2.0    REQUIRED)
pkg_check_modules (OPENCV       opencv          REQUIRED)

include_directories ( ${OPENCV_INCLUDE_DIRS} )
link_directories ( ${OPENCV_LIBRARY_DIRS} )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

set ( SRC_LIST
  ${MCODE_TOP}/tools/converter/main.cpp
)

option ( MCODE_EMULATE_LED "Enable LEDs emulation" ON )

# configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

add_executable( convert-image ${SRC_LIST} )

find_package (Threads)
target_link_libraries ( convert-image ${CMAKE_THREAD_LIBS_INIT} ${OPENCV_LIBRARIES} )

include_directories ( "${PROJECT_BINARY_DIR}/include/" )
