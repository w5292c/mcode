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

#include <alloca.h>
#include <stdarg.h>
#include <avr/pgmspace.h>

static uint32_t TheLcdId = 0;

void lcd_init(uint16_t width, uint16_t height)
{
  hw_i80_init();
  lcd_set_size(width, height);
}

void lcd_deinit(void)
{
  hw_i80_deinit();
}

void lcd_reset(void)
{
  hw_i80_reset();
}

void lcd_read(uint8_t cmd, uint8_t length, uint8_t *data)
{
}

void lcd_set_bl(bool on)
{
}

uint32_t lcd_read_id(void)
{
  uint8_t data[5] = {0};
  hw_i80_read(0xBFU, 5, data);

  if (0xFFU == data[4]) {
    TheLcdId = data[0]; TheLcdId <<= 8;
    TheLcdId |= data[1]; TheLcdId <<= 8;
    TheLcdId |= data[2]; TheLcdId <<= 8;
    TheLcdId |= data[3];
  } else {
    TheLcdId = 0;
    hw_uart_write_string_P(PSTR("lcd_read_id: wrong data\r\n"));
  }

  return TheLcdId;
}

uint16_t lcd_get_width(void)
{
  return 320;
}

uint16_t lcd_get_height(void)
{
  return 480;
}

void lcd_set_size(uint16_t width, uint16_t height)
{
  hw_uart_write_string_P(PSTR("W: lcd_set_size(0x"));
  hw_uart_write_uint16(width, true);
  hw_uart_write_string_P(PSTR(", 0x"));
  hw_uart_write_uint16(height, true);
  hw_uart_write_string_P(PSTR(")\r\n"));
}

void lcd_set_scroll_start(uint16_t start)
{
  lcd_command(0x37, start>>8, start);
}

void lcd_set_window(uint16_t colStart, uint16_t colEnd, uint16_t rowStart, uint16_t rowEnd)
{
  lcd_command(0x2A, colStart>>8, colStart, colEnd>>8, colEnd);
  lcd_command(0x2B, rowStart>>8, rowStart, rowEnd>>8, rowEnd);
}

void lcd_write(int len, ...)
{
  int i;
  uint8_t *data = NULL;
  if (len > 1) {
    data = alloca(len - 1);
  }
  va_list vl;
  va_start(vl, len);
  const uint8_t cmd = (uint8_t)va_arg(vl, unsigned int);
  for (i = 1; i < len; ++i) {
    data[i - 1] = (uint8_t)va_arg(vl, unsigned int);
  }
  va_end(vl);
  hw_i80_write(cmd, len - 1, data);
}

void lcd_write_const_words(uint8_t cmd, uint16_t word, uint32_t count)
{
  hw_i80_write_const_long (cmd, word, count);
}

void lcd_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  hw_i80_write_bitmap(cmd, length, pData, offValue, onValue);
}

void lcd_write_bitmap_P(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  hw_i80_write_bitmap_P(cmd, length, pData, offValue, onValue);
}
