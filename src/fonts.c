#include "fonts.h"

#include "hw-uart.h"
#include "fonts/font8x8_basic.h"

uint8_t mcode_fonts_get_bitmap (uint8_t code, uint8_t line)
{
  uint8_t bitmap = UINT8_C (0x00);

  if (code >= 32 && code <= 127 && line < 8)
  {
    bitmap = pgm_read_byte (&font8x8_basic[code - 32][line]);
  }
  else
  {
    hw_uart_write_string_P (PSTR ("mcode_fonts_get_bitmap: wrong code: ["));
    hw_uart_write_uint (code);
    hw_uart_write_string_P (PSTR ("] OR line: ["));
    hw_uart_write_uint (line);
    hw_uart_write_string_P (PSTR ("]\r\n"));
  }

  return bitmap;
}

const uint8_t *mcode_fonts_get_char_bitmap (uint8_t code)
{
  const uint8_t *pRes = 0;

  if (code >= 32 && code <= 127)
  {
    pRes = &font8x8_basic[code - 32][0];
  }
  else
  {
    hw_uart_write_string_P (PSTR ("mcode_fonts_get_char_bitmap: wrong code: ["));
    hw_uart_write_uint (code);
    hw_uart_write_string_P (PSTR ("]\r\n"));
  }

  return pRes;
}
