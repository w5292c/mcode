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

#include "hw-spi.h"

#include <stdarg.h>

void lcd_write_cmd(uint8_t cmd)
{
  lcd_set_address(false);
  spi_set_cs(true);
  spi_transfer(cmd);
  spi_set_cs(false);
}

void lcd_write_byte(uint8_t data)
{
  lcd_set_address(true);
  spi_set_cs(true);
  spi_transfer(data);
  spi_set_cs(false);
}

void lcd_set_bl(bool on)
{
}

static uint8_t lcd_read_register(uint8_t addr, uint8_t param)
{
  uint8_t data = 0;
  lcd_write_cmd(0xD9U);
  lcd_write_byte(0x10 + param);
  lcd_set_address(false);
  spi_set_cs(true);
  spi_transfer(addr);
  lcd_set_address(true);
  data = spi_transfer(0xffu);
  spi_set_cs(false);
  return data;
}

uint32_t lcd_read_id(void)
{
  int i;
  uint8_t id[3] = {0};
  for (i = 0; i < 3; ++i) {
    id[i] = lcd_read_register(0xd3, i + 1);
  }

  const uint32_t idValue = ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
  return idValue;
}

void lcd_write(int len, ...)
{
  int i;
  va_list vl;
  va_start(vl, len);
  lcd_write_cmd(va_arg(vl, unsigned int));
  for (i = 1; i < len; ++i) {
    lcd_write_byte(va_arg(vl, unsigned int));
  }
  va_end(vl);
}

void lcd_write_const_words(uint8_t cmd, uint16_t word, uint32_t count)
{
  uint32_t i;
  const uint8_t byte1 = word>>8;
  const uint8_t byte2 = word;
  lcd_write_cmd(cmd);
  for (i = 0; i < count; ++i) {
    lcd_write_byte(byte1);
    lcd_write_byte(byte2);
  }
}

void lcd_write_bitmap_P(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  uint8_t bitMask;
  uint8_t currentByte;
  const uint8_t *const pDataEnd = pData + length;

  lcd_write_cmd(cmd);
  /* write loop */
  currentByte = pgm_read_byte(pData++);
  for (bitMask = UINT8_C (0x01); ; ) {
    const uint16_t currentData = (currentByte & bitMask) ? onValue : offValue;
    lcd_write_byte(currentData>>8);
    lcd_write_byte(currentData);
    bitMask = (bitMask << 1);
    if (!bitMask) {
      if (pData < pDataEnd) {
        bitMask = UINT8_C (0x01);
        currentByte = pgm_read_byte(pData++);
      }
      else {
        break;
      }
    }
  }
}
