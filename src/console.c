#include "console.h"

#include "fonts.h"
#include "hw-i80.h"
#include "hw-uart.h"
#include "hw-lcd-s95513.h"

#include <string.h>
#include <avr/pgmspace.h>

static uint8_t TheCurrentLine = 0;
static uint8_t TheCurrentColumn = 0;

static void console_roll_up (void);
static void console_clear_line (uint8_t line);
static uint8_t console_handle_utf8 (uint8_t byte);
static uint8_t console_handle_control_codes (uint8_t byte);
static uint8_t console_handle_escape_sequence (uint8_t byte);
static void console_config_lcd_for_pos (uint8_t column, uint8_t line);
static const char *console_next_num_token (const char *pString, uint8_t *pValue);

static uint16_t TheOnColor = 0xFFFFU;
static uint16_t TheOffColor = 0x0000U;

/* 2104 3210 | 3450 1234 */
/* GGGR RRRR | GGGB BBBB */
static const uint8_t on_buffer[4] PROGMEM = {
  0xE0U, 0xFFU, 0xFFU, 0xFFU
};
static const uint8_t off_buffer[4] PROGMEM = {
  0x00U, 0x00U, 0x00U, 0x00U
};

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

  /* reset the LCD module, this clears the screen */
  hw_i80_reset ();
  hw_lcd_s95513_turn_on ();
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
  if (TheCurrentColumn >= 40)
  {
    ++TheCurrentLine;
    if (TheCurrentLine >= 60)
    {
      /* roll 1 text line up */
      console_roll_up ();
      /* clear the last text line */
      console_clear_line (59);
      TheCurrentLine = 59;
    }
    TheCurrentColumn = 0;
  }

  console_config_lcd_for_pos (TheCurrentColumn, TheCurrentLine);
  /* move to the next column */
  ++TheCurrentColumn;

  /* now we are ready to send the char bitmap to the LCD module */
  uint8_t x, y;
  uint8_t cmd = UINT8_C (0x2C);
  for (y = 0; y < 8; ++y)
  {
    const uint8_t line = mcode_fonts_get_bitmap (byte , y);
    for (x = 0; x < 8; ++x)
    {
      if (line & (1U << (7 - x)))
      {
        /* the pixel is ON */
        hw_i80_write_double (cmd, 2, (const uint8_t *)(&TheOnColor));
      }
      else
      {
        /* the pixel is OFF */
        hw_i80_write_double (cmd, 2, (const uint8_t *)(&TheOffColor));
      }
      cmd = UINT8_C(0x3C);
    }
  }
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
}

void console_clear_line (uint8_t line)
{
}

void console_config_lcd_for_pos (uint8_t column, uint8_t line)
{
  const uint16_t sCol = (uint16_t)((39 - column)<<3);
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
    /**@todo impelement handling the control-code */
  }

  return controlCode;
}

typedef void (*console_escape_handler) (const char *data);
static console_escape_handler console_esc_check_for_end (void);
static void console_escape_ignore (const char *pArgs);
static void console_escape_set_mode (const char *pArgs);
static void console_escape_set_cursor_pos (const char *pArgs);

typedef struct
{
  const char *m_pSuffix;
  console_escape_handler m_pHandler;
} EscapeSequence;

const char EscSuffix0[] PROGMEM = "m";
const char EscSuffix1[] PROGMEM = "H";
const char EscSuffix2[] PROGMEM = "f";
const char EscSuffix3[] PROGMEM = "2J";
const char EscSuffix4[] PROGMEM = "m";
const char EscSuffix5[] PROGMEM = "m";
static const EscapeSequence TheEscapeSequesnceHandlers[] PROGMEM = {
  {EscSuffix0, console_escape_set_mode},
  {EscSuffix1, console_escape_set_cursor_pos},
  {EscSuffix2, console_escape_set_cursor_pos},
  {EscSuffix3, console_escape_ignore},
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
      EscSequenceBuffer[escapeSequenceIndex++] = byte;
      console_escape_handler pHandler = console_esc_check_for_end ();
      if (pHandler)
      {
        (*pHandler) ((const char *)EscSequenceBuffer);
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

console_escape_handler console_esc_check_for_end (void)
{
  uint8_t i;
  console_escape_handler pHandler = NULL;
  const uint8_t bufferLength = strlen ((const char *)EscSequenceBuffer);
  const uint8_t n = sizeof (TheEscapeSequesnceHandlers)/sizeof (TheEscapeSequesnceHandlers[0]);
  for (i = 0; i < n; ++i)
  {
    const EscapeSequence *const pSequence = &TheEscapeSequesnceHandlers[i];
    const char *const pSuffix = (const char *) pgm_read_word (&pSequence->m_pSuffix);
    const uint8_t suffixLength = strlen_P (pSuffix);

    if (bufferLength >= suffixLength && !strcmp_P ((const char *)(EscSequenceBuffer + bufferLength - suffixLength), pSuffix))
    {
      /* retrieve the handler */
      pHandler = (console_escape_handler) pgm_read_word (&pSequence->m_pHandler);
      /* remove the suffix from the buffer */
      *(EscSequenceBuffer + bufferLength - suffixLength) = 0;
      break;
    }
  }

  return pHandler;
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
    *pValue = 0;
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
        *pValue *= 10;
        *pValue += (ch - '0');
      }
    }
  }

  return pNext;
}

void console_escape_set_cursor_pos (const char *pArgs)
{
  hw_uart_write_string_P (PSTR ("> Set cursor position: ["));
  hw_uart_write_string (pArgs);
  hw_uart_write_string_P (PSTR ("]\r\n"));
}
