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

#include "console.h"

#include "fonts.h"
#include "hw-i80.h"
#include "hw-uart.h"
#include "hw-lcd-s95513.h"

#include <string.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

#define CONSOLE_HTAB_SIZE (4)

static int8_t TheSavedLinePos = 0;
static int8_t TheSavedColumnPos = 0;

static int8_t TheCurrentLine = 0;
static int8_t TheCurrentColumn = 0;

static int8_t TheLineCount = 60;
static int8_t TheColumnCount = 40;

/**
 * The current number of lines the screen has been scrolled up.
 */
static uint8_t TheCurrentScrollPos = 0;

static void console_roll_up (void);
static uint8_t console_handle_utf8 (uint8_t byte);
static uint8_t console_handle_control_codes (uint8_t byte);
static uint8_t console_handle_escape_sequence (uint8_t byte);
static void console_config_lcd_for_pos (uint8_t column, uint8_t line);
static const char *console_next_num_token (const char *pString, uint8_t *pValue);
static void console_escape_clear_line (uint8_t line, int8_t startColumn, int8_t endColumn);

/* 2104 3210 | 3450 1234 */
/* GGGR RRRR | GGGB BBBB */
static uint16_t TheOnColor = 0xFFFFU;
static uint16_t TheOffColor = 0x0000U;

void console_init (void)
{
}

void console_deinit (void)
{
}

void console_clear_screen (void)
{
  /* reset the console state */
  TheCurrentLine = 0;
  TheCurrentColumn = 0;

  /* turn the LCD on */
  hw_lcd_s95513_turn_on ();

  /* clear the screen, fill the background color */
  uint8_t buffer[4];
  /* set_column_address */
  buffer[0] = UINT8_C (0x00);
  buffer[1] = UINT8_C (0x00); /* start column */
  buffer[2] = UINT8_C (0x01);
  buffer[3] = UINT8_C (0x3f); /* end column */
  hw_i80_write (UINT8_C(0x2a), 4, buffer);
  /* set_page_address */
  buffer[0] = UINT8_C (0x00);
  buffer[1] = UINT8_C (0x00); /* start page */
  buffer[2] = UINT8_C (0x01);
  buffer[3] = UINT8_C (0xdf); /* end page */
  hw_i80_write (UINT8_C(0x2B), 4, buffer);
  hw_i80_write_const_long (UINT8_C(0x2c), TheOffColor, 153600);
}

void console_write_byte (uint8_t byte)
{
  if (console_handle_escape_sequence (byte))
  {
    return;
  }
  if (console_handle_control_codes (byte))
  {
    return;
  }
  if (console_handle_utf8 (byte))
  {
    return;
  }

  /* check the current position */
  if (TheCurrentColumn >= TheColumnCount)
  {
    ++TheCurrentLine;
    if (TheCurrentLine >= TheLineCount)
    {
      /* roll 1 text line up */
      console_roll_up ();
      /* clear the last text line */
      TheCurrentLine = TheLineCount - 1;
    }
    TheCurrentColumn = 0;
  }

  console_config_lcd_for_pos (TheCurrentColumn, TheCurrentLine);
  /* move to the next column */
  ++TheCurrentColumn;

  /* now we are ready to send the char bitmap to the LCD module */
#if 1
  const uint8_t *const pChar = mcode_fonts_get_char_bitmap (byte);
  hw_i80_write_bitmap_P (UINT8_C (0x2C), 8, pChar, TheOffColor, TheOnColor);
#else
  uint8_t x, y;
  uint8_t cmd = UINT8_C (0x2C);
  for (y = 0; y < 8; ++y)
  {
    const uint8_t line = mcode_fonts_get_bitmap (byte , y);
    for (x = 0; x < 8; ++x)
    {
      if (line & (1U << x))
      {
        /* the pixel is ON */
        hw_i80_write_words (cmd, 1, &TheOnColor);
      }
      else
      {
        /* the pixel is OFF */
        hw_i80_write_words (cmd, 1, &TheOffColor);
      }
      cmd = UINT8_C(0x3C);
    }
  }
#endif
}

void console_write_string (const char *pString)
{
  uint8_t ch;
  while ((ch = *pString++))
  {
    console_write_byte (ch);
  }
}

void console_write_string_P (const char *pString)
{
  uint8_t ch;
  while ((ch = pgm_read_byte (pString++)))
  {
    console_write_byte (ch);
  }
}

void console_set_color (uint16_t color)
{
  TheOnColor = color;
}

void console_set_bg_color (uint16_t color)
{
  TheOffColor = color;
}

void console_roll_up (void)
{
  /* clear the upper line of text */
  console_escape_clear_line (0, -1, -1);

  /* calculate the target scroll position */
  ++TheCurrentScrollPos;
  if (TheCurrentScrollPos >= TheLineCount)
  {
    TheCurrentScrollPos = 0;
  }

  /* now we are ready to scroll to the next line */
  hw_lcd_s95513_set_scroll_start (TheCurrentScrollPos << 3);
}

void console_config_lcd_for_pos (uint8_t column, uint8_t line)
{
  line += TheCurrentScrollPos;
  if (line >= TheLineCount)
  {
    line -= TheLineCount;
  }

  const uint16_t sCol = (uint16_t)(column << 3);
  const uint16_t eCol = sCol + 7;
  const uint16_t sLine = (uint16_t)(line<<3);
  const uint16_t eLine = sLine + 7;

  uint8_t buffer[4];
  /* set_column_address */
  buffer[0] = (uint8_t)(UINT8_C (0xFF) & (sCol>>8));
  buffer[1] = (uint8_t)(UINT8_C (0xFF) & sCol); /* start column */
  buffer[2] = (uint8_t)(UINT8_C (0xFF) & (eCol>>8));
  buffer[3] = (uint8_t)(UINT8_C (0xFF) & eCol); /* end column */
  hw_i80_write (UINT8_C(0x2A), 4, buffer);
  /* set_page_address */
  buffer[0] = (uint8_t)(UINT8_C (0xFF) & (sLine>>8));
  buffer[1] = (uint8_t)(UINT8_C (0xFF) & sLine); /* start page */
  buffer[2] = (uint8_t)(UINT8_C (0xFF) & (eLine>>8));
  buffer[3] = (uint8_t)(UINT8_C (0xFF) & eLine); /* end page */
  hw_i80_write (UINT8_C(0x2B), 4, buffer);
}

uint8_t console_handle_utf8 (uint8_t byte)
{
  /**@todo implement */
  return (byte > 127);
}

uint8_t console_handle_control_codes (uint8_t byte)
{
  const uint8_t controlCode = (byte < 32 && byte != 27);
  if (controlCode)
  {
    switch (byte)
    {
    case '\r':
      TheCurrentColumn = 0;
      break;
    case 9: /*Horizontal Tab */
      {
        const uint8_t originalColumn = TheCurrentColumn;
        TheCurrentColumn = (TheCurrentColumn/CONSOLE_HTAB_SIZE + 1) * CONSOLE_HTAB_SIZE;
        console_escape_clear_line (TheCurrentLine, originalColumn, TheCurrentColumn);
        if (TheCurrentColumn >= TheColumnCount)
        {
          TheCurrentColumn = 0;
          /* fall though to next-line */
        }
        else
        {
          break;
        }
      }
    case 11: /* vertical tab */
    case '\n':
      ++TheCurrentLine;
      if (TheCurrentLine >= TheLineCount)
      {
        /* roll 1 text line up */
        console_roll_up ();
        TheCurrentLine = TheLineCount - 1;
      }
      break;
    case 8: /* backspace */
      if (--TheCurrentColumn < 0)
      {
        TheCurrentColumn = 0;
      }
      console_escape_clear_line (TheCurrentLine, TheCurrentColumn, TheCurrentColumn + 1);
      break;
    default:
      break;
    }
  }

  return controlCode;
}

typedef void (*console_escape_handler) (const char *data);
static uint8_t console_esc_check_for_end (console_escape_handler *pHandler);
static void console_escape_ignore (const char *pArgs);
static void console_escape_set_mode (const char *pArgs);
static void console_escape_cursor_up (const char *pArgs);
static void console_escape_cursor_down (const char *pArgs);
static void console_escape_cursor_forward (const char *pArgs);
static void console_escape_cursor_backward (const char *pArgs);
static void console_escape_erase_line (const char *pArgs);
static void console_escape_clear_screen (const char *pArgs);
static void console_escape_set_cursor_pos (const char *pArgs);
static void console_escape_save_cursor_pos (const char *pArgs);
static void console_escape_restore_cursor_pos (const char *pArgs);

typedef struct
{
  const char *m_pSuffix;
  console_escape_handler m_pHandler;
} EscapeSequence;

const char EscSuffix00[] PROGMEM = "m";  /* set-graphics-mode */
const char EscSuffix01[] PROGMEM = "H";  /* set-cursor-position */
const char EscSuffix02[] PROGMEM = "f";  /* set-cursor-position */
const char EscSuffix03[] PROGMEM = "2J"; /* clear-screen */
const char EscSuffix04[] PROGMEM = "A";  /* cursor-up */
const char EscSuffix05[] PROGMEM = "B";  /* cursor-down */
const char EscSuffix06[] PROGMEM = "C";  /* cursor-forward */
const char EscSuffix07[] PROGMEM = "D";  /* cursor-backward */
const char EscSuffix08[] PROGMEM = "s";  /* Save the current cursor position */
const char EscSuffix09[] PROGMEM = "u";  /* Restore the saved cursor position */
const char EscSuffix10[] PROGMEM = "h";  /* Set mode, ignore for now */
const char EscSuffix11[] PROGMEM = "l";  /* Reset mode, ignore for now */
const char EscSuffix12[] PROGMEM = "p";  /* Set keyboard settings, ignore for now */
const char EscSuffix13[] PROGMEM = "K";  /* Erase line */
static const EscapeSequence TheEscapeSequesnceHandlers[] PROGMEM = {
  {EscSuffix00, console_escape_set_mode},
  {EscSuffix01, console_escape_set_cursor_pos},
  {EscSuffix02, console_escape_set_cursor_pos},
  {EscSuffix03, console_escape_clear_screen},
  {EscSuffix04, console_escape_cursor_up},
  {EscSuffix05, console_escape_cursor_down},
  {EscSuffix06, console_escape_cursor_forward},
  {EscSuffix07, console_escape_cursor_backward},
  {EscSuffix08, console_escape_save_cursor_pos},
  {EscSuffix09, console_escape_restore_cursor_pos},
  {EscSuffix13, console_escape_erase_line},
  {EscSuffix10, console_escape_ignore},
  {EscSuffix11, console_escape_ignore},
  {EscSuffix12, console_escape_ignore},
};
static uint8_t EscSequenceBuffer[16];
uint8_t console_handle_escape_sequence (uint8_t byte)
{
  /**
   * Escape-sequence phase:
   * 0 - Escape-sequence has not been detected;
   * 1 - Escape character has been detected;
   * 2 - Escape sequence has been detected, reading parameters;
   * 3 - Error.
   */
  static uint8_t escapeSequence = 0;
  static uint8_t escapeSequenceIndex = 0;

  uint8_t result = 0;
  switch (escapeSequence)
  {
  case 0:
    if (27 == byte)
    {
      escapeSequence = 1;
      result = 1;
    }
    break;
  case 1:
    if ('[' == byte)
    {
      /* 'ESC[' detected, start recording parameters */
      memset (EscSequenceBuffer, 0, sizeof (EscSequenceBuffer));
      escapeSequence = 2;
      escapeSequenceIndex = 0;
      result = 1;
    }
    break;
  case 2:
    /**@todo At this point we might have received the closing suffix, probably,
             makes sense to check for it before reporting an error */
    if (escapeSequenceIndex < sizeof (EscSequenceBuffer) - 1)
    {
      console_escape_handler pHandler = NULL;
      EscSequenceBuffer[escapeSequenceIndex++] = byte;
      if (console_esc_check_for_end (&pHandler))
      {
        if (pHandler)
        {
          (*pHandler) ((const char *)EscSequenceBuffer);
        }
        escapeSequence = 0;
      }
    }
    else
    {
      /* detected too-long escape sequence, print debug output to UART */
      hw_uart_write_string_P (PSTR ("Error: console: too long escape sequence. Hex dump:\r\n> "));
      uint8_t i;
      for (i = 0; i < sizeof (EscSequenceBuffer); ++i)
      {
        hw_uart_write_string_P (PSTR (" "));
        hw_uart_write_uint (EscSequenceBuffer [i]);
        if (i != (sizeof (EscSequenceBuffer) - 1))
        {
          hw_uart_write_string_P (PSTR (","));
        }
      }
      hw_uart_write_string_P (PSTR ("\r\n"));
      escapeSequence = 3;
    }

    result = 1;
    break;
  default:
    break;
  }

  return result;
}

uint8_t console_esc_check_for_end (console_escape_handler *pHandler)
{
  uint8_t i;
  uint8_t found = 0;
  const uint8_t bufferLength = strlen ((const char *)EscSequenceBuffer);
  const uint8_t n = sizeof (TheEscapeSequesnceHandlers)/sizeof (TheEscapeSequesnceHandlers[0]);
  for (i = 0; i < n; ++i)
  {
    const EscapeSequence *const pSequence = &TheEscapeSequesnceHandlers[i];
#ifdef __AVR__
    const char *const pSuffix = (const char *) pgm_read_word (&pSequence->m_pSuffix);
#else /* __AVR__ */
    const char *const pSuffix = pSequence->m_pSuffix;
#endif /* __AVR__ */
    const uint8_t suffixLength = strlen_P (pSuffix);

    if (bufferLength >= suffixLength && !strcmp_P ((const char *)(EscSequenceBuffer + bufferLength - suffixLength), pSuffix))
    {
      /* retrieve the handler */
#ifdef __AVR__
      *pHandler = (console_escape_handler) pgm_read_word (&pSequence->m_pHandler);
#else /* __AVR__ */
      *pHandler = pSequence->m_pHandler;
#endif /* __AVR__ */
      /* remove the suffix from the buffer */
      *(EscSequenceBuffer + bufferLength - suffixLength) = 0;
      found = 1;
      break;
    }
  }

  return found;
}

void console_escape_ignore (const char *pArgs)
{
  hw_uart_write_string_P (PSTR ("> Ignore: ["));
  hw_uart_write_string (pArgs);
  hw_uart_write_string_P (PSTR ("]\r\n"));
}

void console_escape_set_mode (const char *pArgs)
{
  if (!(*pArgs))
  {
    /* empty arguments, reset the console modes */
    TheOnColor = 0xFFFFU;
    console_set_bg_color (0x0000U);
    return;
  }
  else
  {
    uint8_t value;
    while ((pArgs = console_next_num_token (pArgs, &value)))
    {
      switch (value)
      {
      case 0:
        /* reset text mode */
        TheOnColor = 0xFFFFU;
        console_set_bg_color (0x0000U);
        break;
      case 30:
        /* set foreground color: black */
        TheOnColor = 0x0000U;
        break;
      case 31:
        /* set foreground color: red */
        TheOnColor = 0x001fU;
        break;
      case 32:
        /* set foreground color: red */
        TheOnColor = 0xe0e0U;
        break;
      case 33:
        /* set foreground color: yellow */
        TheOnColor = 0xe0ffU;
        break;
      case 34:
        /* set foreground color: blue */
        TheOnColor = 0x1f00U;
        break;
      case 35:
        /* set foreground color: magenta */
        TheOnColor = 0x1f1fU;
        break;
      case 36:
        /* set foreground color: cyan */
        TheOnColor = 0xffe0U;
        break;
      case 37:
        /* set foreground color: white */
        TheOnColor = 0xffffU;
        break;
      case 40:
        /* set background color: black */
        console_set_bg_color (0x0000U);
        break;
      case 41:
        /* set background color: red */
        console_set_bg_color (0x001fU);
        break;
      case 42:
        /* set background color: green */
        console_set_bg_color (0xe0e0U);
        break;
      case 43:
        /* set background color: yellow */
        console_set_bg_color (0xe0ffU);
        break;
      case 44:
        /* set background color: blue */
        console_set_bg_color (0x1f00U);
        break;
      case 45:
        /* set background color: magenta */
        console_set_bg_color (0x1f1fu);
        break;
      case 46:
        /* set background color: cyan */
        console_set_bg_color (0xffe0U);
        break;
      case 47:
        /* set background color: white */
        console_set_bg_color (0xffffu);
        break;
      default:
        hw_uart_write_string_P (PSTR ("DEBUG: console_escape_set_mode: unknown mode: ["));
        hw_uart_write_string (pArgs);
        hw_uart_write_string_P (PSTR ("]\r\n"));
        break;
      }
    }
  }
}

const char *console_next_num_token (const char *pString, uint8_t *pValue)
{
  const char *pNext = NULL;
  char ch;
  if (*pString)
  {
    if (pValue)
    {
      *pValue = 0;
    }
    while (1)
    {
      ch = *pString++;
      if (!ch)
      {
        /* end-of-string detected */
        pNext = pString;
        break;
      }
      else if (';'== ch)
      {
        /* end-of-token detected */
        pNext = pString;
        break;
      }
      else if (ch >= '0' && ch <= '9')
      {
        /* a decimal digit detected */
        if (pValue)
        {
          *pValue *= 10;
          *pValue += (ch - '0');
        }
      }
    }
  }

  return pNext;
}

void console_escape_set_cursor_pos (const char *pArgs)
{
  uint8_t valueLine = 0;
  pArgs = console_next_num_token (pArgs, &valueLine);
  if (pArgs)
  {
    uint8_t valueColumn = 0;
    pArgs = console_next_num_token (pArgs, &valueColumn);
    if (pArgs && !console_next_num_token (pArgs, NULL))
    {
      if (valueLine < TheLineCount && valueColumn < TheColumnCount)
      {
        TheCurrentLine = valueLine;
        TheCurrentColumn = valueColumn;
      }
      else
      {
        hw_uart_write_string_P (PSTR ("Warning: console_escape_set_cursor_pos: wrong cursor pos: "));
        hw_uart_write_uint (valueColumn);
        hw_uart_write_string_P (PSTR (", "));
        hw_uart_write_uint (valueLine);
        hw_uart_write_string_P (PSTR ("\r\n"));
      }
    }
  }
}

void console_escape_clear_screen (const char *pArgs)
{
  if (!*pArgs)
  {
    console_clear_screen ();
  }
}

void console_escape_cursor_up (const char *pArgs)
{
  uint8_t lines = 0;
  pArgs = console_next_num_token (pArgs, &lines);
  if (pArgs && !console_next_num_token (pArgs, NULL))
  {
    TheCurrentLine -= lines;
    if (TheCurrentLine < 0 || TheCurrentLine >= TheLineCount)
    {
      TheCurrentLine = 0;
    }
  }
}

void console_escape_cursor_down (const char *pArgs)
{
  uint8_t lines = 0;
  pArgs = console_next_num_token (pArgs, &lines);
  if (pArgs && !console_next_num_token (pArgs, NULL))
  {
    TheCurrentLine += lines;
    if (TheCurrentLine >= TheLineCount || TheCurrentLine < 0)
    {
      TheCurrentLine = TheLineCount - 1;
    }
  }
}
void console_escape_cursor_forward (const char *pArgs)
{
  uint8_t columns = 0;
  pArgs = console_next_num_token (pArgs, &columns);
  if (pArgs && !console_next_num_token (pArgs, NULL))
  {
    TheCurrentColumn += columns;
    if (TheCurrentColumn >= TheColumnCount || TheCurrentColumn < 0)
    {
      TheCurrentColumn = TheColumnCount - 1;
    }
  }
}
void console_escape_cursor_backward (const char *pArgs)
{
  uint8_t columns = 0;
  pArgs = console_next_num_token (pArgs, &columns);
  if (pArgs && !console_next_num_token (pArgs, NULL))
  {
    TheCurrentColumn -= columns;
    if (TheCurrentColumn < 0 || TheCurrentColumn >= TheColumnCount)
    {
      TheCurrentColumn = 0;
    }
  }
}

void console_escape_save_cursor_pos (const char *pArgs)
{
  if (!(*pArgs))
  {
    TheSavedLinePos = TheCurrentLine;
    TheSavedColumnPos = TheCurrentColumn;
  }
}

void console_escape_restore_cursor_pos (const char *pArgs)
{
  if (!(*pArgs))
  {
    TheCurrentLine = TheSavedLinePos;
    TheCurrentColumn = TheSavedColumnPos;

    TheSavedLinePos = 0;
    TheSavedColumnPos = 0;
  }
}

void console_escape_erase_line (const char *pArgs)
{
  if (!*pArgs)
  {
    console_escape_clear_line (TheCurrentLine, TheCurrentColumn, -1);
  }
}

void console_escape_clear_line (uint8_t line, int8_t startColumn, int8_t endColumn)
{
  if (startColumn < 0)
  {
    startColumn = 0;
  }
  if (endColumn < 0)
  {
    endColumn = TheColumnCount;
  }
  line += TheCurrentScrollPos;
  if (line >= TheLineCount)
  {
    line -= TheLineCount;
  }

  const uint16_t sCol = (uint16_t)(startColumn << 3);
  const uint16_t eCol = (((uint16_t)endColumn) << 3) - 1;
  const uint16_t sLine = (uint16_t)(line << 3);
  const uint16_t eLine = sLine + 7;

  /* clear the line, fill the background color */
  uint8_t buffer[4];
  /* set_column_address */
  buffer[0] = (uint8_t)(UINT8_C (0xFF) & (sCol>>8));
  buffer[1] = (uint8_t)(UINT8_C (0xFF) & sCol); /* start column */
  buffer[2] = (uint8_t)(UINT8_C (0xFF) & (eCol>>8));
  buffer[3] = (uint8_t)(UINT8_C (0xFF) & eCol); /* end column: 0x013f */
  hw_i80_write (UINT8_C(0x2A), 4, buffer);
  /* set_page_address */
  buffer[0] = (uint8_t)(UINT8_C (0xFF) & (sLine>>8));
  buffer[1] = (uint8_t)(UINT8_C (0xFF) & sLine); /* start page */
  buffer[2] = (uint8_t)(UINT8_C (0xFF) & (eLine>>8));
  buffer[3] = (uint8_t)(UINT8_C (0xFF) & eLine); /* end page */
  hw_i80_write (UINT8_C(0x2B), 4, buffer);
  hw_i80_write_const (UINT8_C(0x2c), TheOffColor, ((TheColumnCount - TheCurrentColumn) << 6));
}
