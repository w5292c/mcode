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

void lcd_reset(void)
{
  hw_i80_reset();

  /* wait for LCD ready */
  while (0x02049481L != lcd_read_id());

  /* initialize the LCD module */
  lcd_device_init();
}

void lcd_write_const_words(uint8_t cmd, uint16_t word, uint32_t count)
{
  hw_i80_write_const_long(cmd, word, count);
}

void lcd_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  hw_i80_write_bitmap(cmd, length, pData, offValue, onValue);
}
