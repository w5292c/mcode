cmake_minimum_required (VERSION 2.6)

project (console-test)

find_package ( Threads )
find_package ( Qt4       REQUIRED )
find_package ( PkgConfig REQUIRED )

pkg_check_modules ( GLIB2_0       glib-2.0        REQUIRED )
pkg_check_modules ( SQLITE        sqlite3         REQUIRED )
link_directories ( ${GLIB2_0_LIBRARY_DIRS} ${SQLITE_LIBRARY_DIRS} )

set ( MCODE_TOP ${CMAKE_SOURCE_DIR}/../../ )

include_directories (
  ${GLIB2_0_INCLUDE_DIRS}
  ${SQLITE_INCLUDE_DIRS}
  ${MCODE_TOP}/src/
  ${MCODE_TOP}/src/common/
  ${MCODE_TOP}/src/security/
)

INCLUDE ( ${QT_USE_FILE} )
ADD_DEFINITIONS ( ${QT_DEFINITIONS} )
ADD_DEFINITIONS ( -D__STDC_CONSTANT_MACROS -Wreturn-type -Wall -Werror )

set ( SRC_LIST
  ${MCODE_TOP}/src/emu/main.cpp
  ${MCODE_TOP}/src/common/mvars.c
  ${MCODE_TOP}/src/common/utils.c
  ${MCODE_TOP}/src/common/hw-lcd.c
  ${MCODE_TOP}/src/common/mtimer.c
  ${MCODE_TOP}/src/common/console.c
  ${MCODE_TOP}/src/common/cmd-lcd.c
  ${MCODE_TOP}/src/common/cmd-ssl.c
  ${MCODE_TOP}/src/common/hw-uart.c
  ${MCODE_TOP}/src/common/mstatus.c
  ${MCODE_TOP}/src/common/mstring.c
  ${MCODE_TOP}/src/common/mparser.c
  ${MCODE_TOP}/src/common/cmd-help.c
  ${MCODE_TOP}/src/common/cmd-switch.c
  ${MCODE_TOP}/src/common/cmd-system.c
  ${MCODE_TOP}/src/common/cmd-console.c
  ${MCODE_TOP}/src/common/cmd-engine.c
  ${MCODE_TOP}/src/common/hw-lcd-i80.c
  ${MCODE_TOP}/src/common/cmd-test-image.c
  ${MCODE_TOP}/src/common/line-editor-uart.c
  ${MCODE_TOP}/src/fonts.c
  ${MCODE_TOP}/src/emu/mtick.c
  ${MCODE_TOP}/src/emu/system.c
  ${MCODE_TOP}/src/emu/hw-leds.c
  ${MCODE_TOP}/src/emu/hw-uart.c
  ${MCODE_TOP}/src/emu/scheduler.c
  ${MCODE_TOP}/src/emu/switch-engine.c
  ${MCODE_TOP}/src/emu/hw-lcd-s95513.cpp
)

set ( SRC_LIST_SIM
  ${MCODE_TOP}/src/emu/main-sim.c
)

set ( SRC_LIST_QT
  ${MCODE_TOP}/src/emu/customwidget.cpp
)

set ( HEADERS_LIST_QT
  ${MCODE_TOP}/src/emu/customwidget.h
)

QT4_WRAP_CPP ( HEADERS_MOC ${HEADERS_LIST_QT} )

option ( MCODE_UART2 "Enable UART2" ON )
option ( MCODE_GSM "Enable GSM Engine" ON )
option ( MCODE_LCD "Enable LCD support" ON )
option ( MCODE_PROG "Enable programming commands" ON )
option ( MCODE_SECURITY "Enable security support" ON )
option ( MCODE_NEW_ENGINE "Enable new Command Engine" ON )
option ( MCODE_TEST_IMAGES "Enable hard-coded test images" ON )
option ( MCODE_COMMAND_MODES "Enable command engine modes" ON )
option ( MCODE_DEBUG_BLINKING "Enable debug LEDs blinking" OFF )
option ( MCODE_HW_I80_ENABLED "HW I80 interface is enabled" ON )
option ( MCODE_CONSOLE_ENABLED "Concole implementation exists" ON )

option ( MCODE_PERSIST_STORE "Enable persistent store" ON )
option ( MCODE_PERSIST_STORE_SQL "Enable SQL persistent store" ON )
option ( MCODE_PERSIST_STORE_FAKE "Enable fake persistent store" OFF )

if ( MCODE_SECURITY )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/security/librock_sha256.c
  )
endif ( MCODE_SECURITY )

if ( MCODE_PERSIST_STORE_FAKE )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/emu/persistent-store.c
  )
endif ( MCODE_PERSIST_STORE_FAKE )

if ( MCODE_PERSIST_STORE_SQL )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/emu/persistent-store-sql.c
  )
endif ( MCODE_PERSIST_STORE_SQL )

if ( MCODE_GSM )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-gsm.c
    ${MCODE_TOP}/src/emu/gsm-engine-uart2.c
    ${MCODE_TOP}/src/common/gsm-engine-uart2.c
  )
endif ( MCODE_GSM )

if ( MCODE_PROG )
  set ( SRC_LIST ${SRC_LIST}
    ${MCODE_TOP}/src/common/cmd-prog.c
    ${MCODE_TOP}/src/emu/hw-nvm.c
  )
endif ( MCODE_PROG )

configure_file ( ${MCODE_TOP}/mcode-config.h.in ${PROJECT_BINARY_DIR}/include/mcode-config.h )

add_executable( console-test ${SRC_LIST} ${HEADERS_MOC} ${SRC_LIST_QT} )
target_link_libraries ( console-test ${GLIB2_0_LIBRARIES} ${SQLITE_LIBRARIES} )
target_link_libraries ( console-test ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries ( console-test  ${QT_LIBRARIES} )

include_directories ( "${PROJECT_BINARY_DIR}/include/" )

add_executable ( mcode-simulator ${SRC_LIST_SIM} )
target_link_libraries ( mcode-simulator ${CMAKE_THREAD_LIBS_INIT} )

add_custom_command (
  TARGET console-test
  POST_BUILD
  COMMAND ../data/copy.sh
  WORKING_DIRECTORY ${MCODE_TOP}/data
  COMMENT "Making sure the store.db exists in the home directory"
)
