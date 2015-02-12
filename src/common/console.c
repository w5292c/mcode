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

#include "utils.h"
#include "fonts.h"
#include "hw-lcd.h"
#include "hw-uart.h"

#include <string.h>

#define CONSOLE_HTAB_SIZE (4)

static int8_t TheSavedLinePos = 0;
static int8_t TheSavedColumnPos = 0;
static int8_t TheSavedScrollPos = 0;

static int8_t TheCurrentLine = 0;
static int8_t TheCurrentColumn = 0;

static uint8_t TheLineCount = 0;
static uint8_t TheColumnCount = 0;

static uint16_t TheDisplayWidth = 0;
static uint16_t TheDisplayHeight = 0;

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

/* 4321 0543 | 2104 3210 */
/* RRRR RGGG | GGGB BBBB */
static uint16_t TheOnColor = UINT16_C(0xFFFF);
static uint16_t TheOffColor = UINT16_C(0x0000);

void console_init(void)
{
  TheDisplayWidth = lcd_get_width();
  TheDisplayHeight = lcd_get_height();
  TheLineCount = TheDisplayHeight>>3;
  TheColumnCount = TheDisplayWidth>>3;
}

void console_deinit(void)
{
}

void console_clear_screen(void)
{
  /* Reset the current scroll position */
  lcd_set_scroll_start(0);
  TheCurrentScrollPos = 0;

  /* reset the console state */
  TheCurrentLine = 0;
  TheCurrentColumn = 0;

  /* turn the LCD on */
  lcd_turn(true);

  /* clear the screen, fill the background color */
  lcd_cls(TheOffColor);
}

void console_write_byte (uint8_t byte)
{
  if (console_handle_escape_sequence (byte)) {
    return;
  }
  if (console_handle_control_codes (byte)) {
    return;
  }
  if (console_handle_utf8 (byte)) {
    return;
  }

  /* check the current position */
  if (TheCurrentColumn >= TheColumnCount) {
    ++TheCurrentLine;
    if (TheCurrentLine >= TheLineCount) {
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
  const uint8_t *const pChar = mcode_fonts_get_char_bitmap (byte);
  lcd_write_bitmap(UINT8_C (0x2C), 8, pChar, TheOffColor, TheOnColor);
}

void console_write_string(const char *pString)
{
  uint8_t ch;
  while ((ch = *pString++)) {
    console_write_byte (ch);
  }
}

void console_write_string_P(const char *pString)
{
#ifdef __AVR__
  uint8_t ch;
  while ((ch = pgm_read_byte((const unsigned char *)(pString++)))) {
    console_write_byte (ch);
  }
#else /* __AVR__ */
  console_write_string(pString);
#endif /* __AVR__ */
}

void console_write_uint16(uint16_t value, bool skipZeros)
{
  int i;
  char buffer[5];
  buffer[0] = nibble_to_char(0x0FU & (value >> 12));
  buffer[1] = nibble_to_char(0x0FU & (value >>  8));
  buffer[2] = nibble_to_char(0x0FU & (value >>  4));
  buffer[3] = nibble_to_char(0x0FU & value);
  buffer[4] = 0;
  if (skipZeros) {
    for (i = 0; i < 3; ++i) {
      if ('0' == *buffer) {
        memmove(buffer, buffer + 1, 4);
      }
    }
  }
  console_write_string(buffer);
}

void console_write_uint32(uint32_t value, bool skipZeros)
{
  const uint16_t upper = (uint16_t)(value>>16);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    console_write_uint16(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  console_write_uint16((uint16_t)value, skipZeros);
}

void console_write_uint64(uint64_t value, bool skipZeros)
{
  const uint32_t upper = (uint32_t)(value>>32);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    console_write_uint32(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  console_write_uint32((uint32_t)value, skipZeros);
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
  lcd_set_scroll_start(TheCurrentScrollPos << 3);
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
  lcd_set_window(sCol, eCol, sLine, eLine);
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
    TheOnColor = UINT16_C(0xFFFF);
    console_set_bg_color(UINT16_C(0x0000));
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
        TheOnColor = UINT16_C(0xFFFF);
        console_set_bg_color(UINT16_C(0x0000));
        break;
      case 30:
        /* set foreground color: black */
        TheOnColor = UINT16_C(0x0000);
        break;
      case 31:
        /* set foreground color: red */
        TheOnColor = UINT16_C(0xF800);
        break;
      case 32:
        /* set foreground color: green */
        TheOnColor = UINT16_C(0x07E0);
        break;
      case 33:
        /* set foreground color: yellow */
        TheOnColor = UINT16_C(0xFFE0);
        break;
      case 34:
        /* set foreground color: blue */
        TheOnColor = UINT16_C(0x001F);
        break;
      case 35:
        /* set foreground color: magenta */
        TheOnColor = UINT16_C(0xF81F);
        break;
      case 36:
        /* set foreground color: cyan */
        TheOnColor = UINT16_C(0x07FF);
        break;
      case 37:
        /* set foreground color: white */
        TheOnColor = UINT16_C(0xFFFF);
        break;
      case 40:
        /* set background color: black */
        console_set_bg_color(UINT16_C(0x0000));
        break;
      case 41:
        /* set background color: red */
        console_set_bg_color(UINT16_C(0xF800));
        break;
      case 42:
        /* set background color: green */
        console_set_bg_color(UINT16_C(0x07E0));
        break;
      case 43:
        /* set background color: yellow */
        console_set_bg_color(UINT16_C(0xFFE0));
        break;
      case 44:
        /* set background color: blue */
        console_set_bg_color(UINT16_C(0x001F));
        break;
      case 45:
        /* set background color: magenta */
        console_set_bg_color(UINT16_C(0xF81F));
        break;
      case 46:
        /* set background color: cyan */
        console_set_bg_color(UINT16_C(0x07FF));
        break;
      case 47:
        /* set background color: white */
        console_set_bg_color(UINT16_C(0xFFFF));
        break;
      default:
        hw_uart_write_string_P(PSTR("DEBUG: console_escape_set_mode: unknown mode: ["));
        hw_uart_write_string(pArgs);
        hw_uart_write_string_P(PSTR("]\r\n"));
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
void console_escape_cursor_backward(const char *pArgs)
{
  uint8_t columns = 0;
  pArgs = console_next_num_token (pArgs, &columns);
  if (pArgs && !console_next_num_token (pArgs, NULL)) {
    TheCurrentColumn -= columns;
    if (TheCurrentColumn < 0 || TheCurrentColumn >= TheColumnCount) {
      TheCurrentColumn = 0;
    }
  }
}

void console_escape_save_cursor_pos(const char *pArgs)
{
  if (!(*pArgs)) {
    TheSavedLinePos = TheCurrentLine;
    TheSavedColumnPos = TheCurrentColumn;
    TheSavedScrollPos = TheCurrentScrollPos;
  }
}

void console_escape_restore_cursor_pos(const char *pArgs)
{
  if (!(*pArgs)) {
    TheCurrentLine = TheSavedLinePos;
    TheCurrentColumn = TheSavedColumnPos;
    const int8_t diffScrollPos = TheCurrentScrollPos - TheSavedScrollPos;
    TheCurrentLine -= diffScrollPos;
    if (TheCurrentLine < 0) {
      /*! @todo Check if this is possible to enter here, if yes, impelement proper handling */
      hw_uart_write_string_P(PSTR("E: console_escape_restore_cursor_pos: wrong line: 0x"));
      hw_uart_write_uint32(TheCurrentLine, false);
      hw_uart_write_string_P(PSTR("\r\n"));
      TheCurrentLine = 0;
    }
    if (TheCurrentLine >= TheLineCount) {
      /*! @todo Check if this is possible to enter here, if yes, impelement proper handling */
      hw_uart_write_string_P(PSTR("E: console_escape_restore_cursor_pos: wrong line: 0x"));
      hw_uart_write_uint32(TheCurrentLine, false);
      hw_uart_write_string_P(PSTR("\r\n"));
      TheCurrentLine = TheLineCount - 1;
    }

    TheSavedLinePos = 0;
    TheSavedColumnPos = 0;
    TheSavedScrollPos = 0;
  }
}

void console_escape_erase_line(const char *pArgs)
{
  if (!*pArgs) {
    console_escape_clear_line (TheCurrentLine, TheCurrentColumn, -1);
  }
}

void console_escape_clear_line(uint8_t line, int8_t startColumn, int8_t endColumn)
{
  if (startColumn < 0) {
    startColumn = 0;
  }
  if (endColumn < 0) {
    endColumn = TheColumnCount;
  }
  line += TheCurrentScrollPos;
  if (line >= TheLineCount) {
    line -= TheLineCount;
  }

  const uint16_t sCol = (uint16_t)(startColumn << 3);
  const uint16_t eCol = (((uint16_t)endColumn) << 3) - 1;
  const uint16_t sLine = (uint16_t)(line << 3);
  const uint16_t eLine = sLine + 7;
  lcd_set_window(sCol, eCol, sLine, eLine);
  lcd_write_const_words(UINT8_C(0x2C), TheOffColor, ((endColumn - startColumn) << 6));
}
