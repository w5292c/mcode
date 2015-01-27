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

void lcd_init(void)
{
}

void lcd_deinit(void)
{
}

void lcd_reset(void)
{
  hw_i80_reset();
}

void lcd_read(uint8_t cmd, uint8_t length)
{
}

void lcd_set_bl(bool on)
{
}

uint32_t lcd_read_id(void)
{
  return 0;
}

uint16_t lcd_get_width(void)
{
  return 240;
}

uint16_t lcd_get_height(void)
{
  return 320;
}

void lcd_set_scroll_start(uint16_t start)
{
  uint8_t buffer[2];
  buffer[0] = (uint8_t)(start>>8);
  buffer[1] = (uint8_t)(start);
  hw_i80_write(UINT8_C(0x37), 2, buffer);
}

void lcd_set_columns(uint16_t start, uint16_t end)
{
}

void lcd_set_pages(uint16_t start, uint16_t end)
{
}

void lcd_write_const_words(uint8_t cmd, uint16_t word, uint32_t count)
{
  hw_i80_write_const_long (cmd, word, count);
}

void lcd_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  hw_i80_write_bitmap(cmd, length, pData, offValue, onValue);
}
