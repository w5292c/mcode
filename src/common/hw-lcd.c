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

#include <stdarg.h>

#ifdef AVR_MEGA
/*#include <avr/io.h>*/
#include <avr/pgmspace.h>
#endif /* AVR_MEGA */

void lcd_cls(uint16_t color)
{
  const uint32_t width = lcd_get_width();
  const uint32_t height = lcd_get_height();
  lcd_set_window(0, width - 1, 0, height - 1);
  lcd_write_const_words(0x2C, color, width*height);
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

void lcd_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  uint8_t bitMask;
  uint8_t currentByte;
  const uint8_t *const pDataEnd = pData + length;

  lcd_write_cmd(cmd);
  /* write loop */
#ifdef AVR_MEGA
  currentByte = pgm_read_byte(pData++);
#else /* AVR_MEGA */
  currentByte = *pData++;
#endif /* AVR_MEGA */
  for (bitMask = UINT8_C(0x01); ; ) {
    const uint16_t currentData = (currentByte & bitMask) ? onValue : offValue;
    lcd_write_byte(currentData>>8);
    lcd_write_byte(currentData);
    bitMask = (bitMask << 1);
    if (!bitMask) {
      if (pData < pDataEnd) {
        bitMask = UINT8_C(0x01);
#ifdef AVR_MEGA
        currentByte = pgm_read_byte(pData++);
#else /* AVR_MEGA */
        currentByte = *pData++;
#endif /* AVR_MEGA */
      }
      else {
        break;
      }
    }
  }
}
