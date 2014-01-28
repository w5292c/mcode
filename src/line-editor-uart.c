#include "line-editor-uart.h"

#include "hw-uart.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

#define LINE_EDITOR_UART_BUFFER_LENGTH (64)
static char line_editor_buffer[LINE_EDITOR_UART_BUFFER_LENGTH] = {0};
static int line_editor_cursor = 0;
static int line_editor_initialized = 0;
static line_editor_uart_ready TheCallback = 0;

static void line_editor_uart_callback (unsigned int aChar);

void line_editor_uart_init (void)
{
  if (!line_editor_initialized)
  {
    memset (line_editor_buffer, 0, LINE_EDITOR_UART_BUFFER_LENGTH);
    hw_uart_set_callback (line_editor_uart_callback);
    line_editor_initialized = 1;
  }
}

void line_editor_uart_deinit (void)
{
  if (line_editor_initialized)
  {
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

void line_editor_uart_callback (unsigned int aChar)
{
  if (line_editor_cursor >= 0)
  {
    /* there is enough space for appending another character */
    if (10 != aChar && 13 != aChar)
    {
      /* non-enter character, check if it is printable and append to the buffer */
      if (aChar < 256 && isprint (aChar))
      {
        /* we have a alpha-numeric character, append it to the buffer */
        if (line_editor_cursor < LINE_EDITOR_UART_BUFFER_LENGTH - 1)
        {
          line_editor_buffer[line_editor_cursor] = (char)aChar;
          hw_uart_write_string (&line_editor_buffer[line_editor_cursor]);
          ++line_editor_cursor;
        }
        else
        {
          hw_uart_write_string ("\007");
        }
      }
      else if (127 == aChar || 8 == aChar)
      {
        /* 'delete' character */
        if (line_editor_cursor > 0)
        {
          --line_editor_cursor;
          line_editor_buffer[line_editor_cursor] = 0;
          hw_uart_write_string_P (PSTR("\010 \010"));
        }
      }
    }
    else
    {
      /* 'enter' character */
      hw_uart_write_string_P (PSTR("\r\n"));
      line_editor_buffer[line_editor_cursor] = 0;
      if (TheCallback)
      {
        (*TheCallback) (line_editor_buffer);
      }
      line_editor_cursor = 0;
      memset (line_editor_buffer, 0, LINE_EDITOR_UART_BUFFER_LENGTH);
    }
  }
  else
  {
    /* negative index, some errors have been happened, wait for 'enter' in order to reset recording */
    if (10 == aChar || 13 == aChar)
    {
      line_editor_cursor = 0;
      memset (line_editor_buffer, 0, LINE_EDITOR_UART_BUFFER_LENGTH);
    }
  }
}
