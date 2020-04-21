# The MIT License (MIT)
#
# Copyright (c) 2020 Alexander Chumakov
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

include ( ${CMAKE_CURRENT_SOURCE_DIR}/../common/mcode-common.cmake )

FUNCTION ( mcode_setup_env )
  if ( NOT DEFINED CMAKE_BUILD_TYPE )
    set ( CMAKE_BUILD_TYPE "Release" )
    set ( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} PARENT_SCOPE )
    message ( "--- (MCODE) Updated MAKE_BUILD_TYPE to ${CMAKE_BUILD_TYPE}" )
  endif ()
  if ( NOT DEFINED STM32_CHIP )
    # Use 'STM32F103C8' chip on default
    set ( STM32_CHIP "STM32F103C8" )
    set ( STM32_CHIP ${STM32_CHIP} PARENT_SCOPE )
    message ( "--- (MCODE) Updated STM32_CHIP to ${STM32_CHIP}" )
  endif ()
  if ( NOT DEFINED STM32_CMAKE_PATH )
    set ( STM32_CMAKE_PATH $ENV{STM32_CMAKE_PATH} )
    message ( "--- (MCODE) Updated STM32_CMAKE_PATH to point to ${STM32_CMAKE_PATH}" )
  endif ()
  if ( NOT DEFINED CMAKE_MODULE_PATH )
    set ( CMAKE_MODULE_PATH "${STM32_CMAKE_PATH}/cmake/Modules" )
    set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE )
    message ( "--- (MCODE) Updated CMAKE_MODULE_PATH to point to ${CMAKE_MODULE_PATH} " )
  endif ()
  if ( NOT DEFINED CMAKE_TOOLCHAIN_FILE )
    set ( CMAKE_TOOLCHAIN_FILE "${STM32_CMAKE_PATH}/gcc_stm32.cmake" )
    set ( CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE} PARENT_SCOPE )
    message ( "--- (MCODE) Updated CMAKE_TOOLCHAIN_FILE to point to ${CMAKE_TOOLCHAIN_FILE}" )
  endif ()
ENDFUNCTION ( mcode_setup_env )
