/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Alexander Chumakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "line-editor-uart.h"

#include "mglobal.h"
#include "hw-uart.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define LINE_EDITOR_UART_BUFFER_LENGTH (64)
static char line_editor_buffer[LINE_EDITOR_UART_BUFFER_LENGTH] = {0};
static int line_editor_cursor = 0;
static int line_editor_initialized = 0;
static line_editor_uart_ready TheCallback = 0;

static void line_editor_uart_callback(char aChar);

void line_editor_uart_init (void)
{
  if (!line_editor_initialized) {
    memset (line_editor_buffer, 0, LINE_EDITOR_UART_BUFFER_LENGTH);
    hw_uart_set_callback (line_editor_uart_callback);
    line_editor_initialized = 1;
  }
}

void line_editor_uart_deinit (void)
{
  if (line_editor_initialized) {
    line_editor_initialized = 0;
    hw_uart_set_callback (0);
    hw_uart_deinit ();
  }
}

void line_editor_uart_set_callback (line_editor_uart_ready aCallback)
{
  TheCallback = aCallback;
}

void line_editor_uart_start (void)
{
  hw_uart_write_string_P (PSTR("# "));
}

void line_editor_uart_callback(char aChar)
{
  if (line_editor_cursor >= 0) {
    /* there is enough space for appending another character */
    if (10 != aChar && 13 != aChar) {
      /* non-enter character, check if it is printable and append to the buffer */
      if (isprint((int)aChar)) {
        /* we have a alpha-numeric character, append it to the buffer */
        if (line_editor_cursor < LINE_EDITOR_UART_BUFFER_LENGTH - 1) {
          line_editor_buffer[line_editor_cursor] = (char)aChar;
          hw_uart_write_string (&line_editor_buffer[line_editor_cursor]);
          ++line_editor_cursor;
        }
        else {
          hw_uart_write_string ("\007");
        }
      }
      else if (127 == aChar || 8 == aChar) {
        /* 'delete' character */
        if (line_editor_cursor > 0) {
          --line_editor_cursor;
          line_editor_buffer[line_editor_cursor] = 0;
          hw_uart_write_string_P (PSTR("\010 \010"));
        }
      }
    }
    else {
      /* 'enter' character */
      hw_uart_write_string_P (PSTR("\r\n"));
      line_editor_buffer[line_editor_cursor] = 0;
      if (TheCallback) {
        (*TheCallback) (line_editor_buffer);
      }
      line_editor_cursor = 0;
      memset (line_editor_buffer, 0, LINE_EDITOR_UART_BUFFER_LENGTH);
    }
  }
  else {
    /* negative index, some errors have been happened, wait for 'enter' in order to reset recording */
    if (10 == aChar || 13 == aChar) {
      line_editor_cursor = 0;
      memset (line_editor_buffer, 0, LINE_EDITOR_UART_BUFFER_LENGTH);
    }
  }
}
