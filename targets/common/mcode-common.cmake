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

FUNCTION ( enable_git_version enable )
  IF ( ${enable} EQUAL 0 )
    message ( "-- No build-time git hash" )
  ELSE ()
    execute_process (
      COMMAND git log -1
      COMMAND grep "commit"
      COMMAND gawk "{ print $2 }"
      OUTPUT_VARIABLE MCODE_GIT_HASH_INTERNAL
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process (
      COMMAND git status --porcelain
      OUTPUT_VARIABLE MCODE_DIRTY
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if ( "x${MCODE_DIRTY}" STREQUAL "x" )
      set ( MCODE_DIRTY_INTERNAL "-(C)" )
    else ()
      set ( MCODE_DIRTY_INTERNAL "-(*)" )
    endif ()
    set ( MCODE_GIT_HASH "${MCODE_GIT_HASH_INTERNAL}${MCODE_DIRTY_INTERNAL}" )
    # Propagate the 'MCODE_GIT_HASH' to the parent scope
    set ( MCODE_GIT_HASH ${MCODE_GIT_HASH} PARENT_SCOPE )
    message ( "-- Git hash: ${MCODE_GIT_HASH}" )
  ENDIF ()
ENDFUNCTION ( enable_git_version )

FUNCTION ( enable_random_numbers COUNT )
  IF ( ${COUNT} EQUAL 0 )
    message ( "-- No build-time random data" )
  ELSE ()
    message ( "-- Build-time random data support: ${COUNT} bytes" )
    set ( MCODE_RANDOM_DATA ON BOOL PARENT_SCOPE )
    execute_process (
      COMMAND dd if=/dev/urandom count=1 bs=${COUNT} status=none
      COMMAND hexdump -v -f ${MCODE_TOP}/hexdump.format
      OUTPUT_VARIABLE MCODE_RANDOM_BYTES
    )
    set ( MCODE_RANDOM_BYTES_COUNT ${COUNT} PARENT_SCOPE )
    set ( MCODE_RANDOM_BYTES ${MCODE_RANDOM_BYTES} PARENT_SCOPE )
    message ( "-- Random bytes: ${MCODE_RANDOM_BYTES}" )
  ENDIF ()
ENDFUNCTION ( enable_random_numbers )

FUNCTION ( publish_phone )
  if ( NOT DEFINED MCODE_DEFAULT_PHONE_NUMBER )
    set ( MCODE_DEFAULT_PHONE_NUMBER "+70001112233" PARENT_SCOPE )
  endif ()
ENDFUNCTION ( publish_phone )
