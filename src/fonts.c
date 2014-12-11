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
