#include "console.h"

#include "fonts.h"
#include "hw-i80.h"
#include "hw-lcd-s95513.h"

#include <string.h>
#include <avr/pgmspace.h>

static uint8_t TheCurrentLine = 0;
static uint8_t TheCurrentColumn = 0;

static void console_roll_up (void);
static void console_clear_line (uint8_t line);
static void console_config_lcd_for_pos (uint8_t column, uint8_t line);

/* GGGR RRRR */
static const uint8_t on_buffer[4] PROGMEM = {
  0xFFU, 0xFFU, 0xFFU, 0xFFU
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
  if (byte < 32 || byte > 127)
  {
    /* no support for now, to be implemented later */
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
        hw_i80_write_P (cmd, 2, on_buffer);
      }
      else
      {
        /* the pixel is OFF */
        hw_i80_write_P (cmd, 2, off_buffer);
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
