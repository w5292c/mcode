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
