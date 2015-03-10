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

static uint8_t lcd_read_register(uint8_t addr, uint8_t param);

uint16_t lcd_get_width(void)
{
  return UINT16_C(240);
}

uint16_t lcd_get_height(void)
{
  return UINT16_C(320);
}

void lcd_turn(bool on)
{
  lcd_command(on ? UINT8_C(0x29) : UINT8_C(0x28));
}

uint32_t lcd_read_id(void)
{
  int i;
  uint8_t id[3] = {0};
  for (i = 0; i < 3; ++i) {
    id[i] = lcd_read_register(UINT8_C(0xD3), i + 1);
  }

  const uint32_t idValue = ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
  return idValue;
}

uint8_t lcd_read_register(uint8_t addr, uint8_t param)
{
  lcd_command(UINT8_C(0xD9), UINT8_C(0x10) + param);
  return lcd_read_byte(addr);
}
