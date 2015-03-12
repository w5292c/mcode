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

#include "hw-lcd.h"

#include "hw-i80.h"
#include "hw-uart.h"

#include <avr/pgmspace.h>

void lcd_init(uint16_t width, uint16_t height)
{
  hw_i80_init();
  lcd_set_size(width, height);
}

void lcd_deinit(void)
{
  hw_i80_deinit();
}

uint32_t lcd_read_id(void)
{
  uint8_t data[5] = {0};
  hw_i80_read(UINT8_C(0xBF), 5, data);

  uint32_t lcdId = 0;
  if (UINT8_C(0xFF) == data[4]) {
    lcdId  = data[0]; lcdId <<= 8;
    lcdId |= data[1]; lcdId <<= 8;
    lcdId |= data[2]; lcdId <<= 8;
    lcdId |= data[3];
  } else {
    hw_uart_write_string_P(PSTR("lcd_read_id: wrong data\r\n"));
  }

  return lcdId;
}

void lcd_set_size(uint16_t width, uint16_t height)
{
  hw_uart_write_string_P(PSTR("W: lcd_set_size(0x"));
  hw_uart_write_uint16(width, true);
  hw_uart_write_string_P(PSTR(", 0x"));
  hw_uart_write_uint16(height, true);
  hw_uart_write_string_P(PSTR(")\r\n"));
}
