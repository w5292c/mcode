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

uint16_t lcd_get_width(void)
{
  return UINT16_C(320);
}

uint16_t lcd_get_height(void)
{
  return UINT16_C(480);
}

void lcd_turn(bool on)
{
  if (on) {
    lcd_device_init();
  } else {
    lcd_command(on ? UINT8_C(0x29) : UINT8_C(0x28));
  }
}

void lcd_device_init(void)
{
  /* exit_sleep_mode */
  lcd_command(0x11);
  lcd_command(0x50);
  /* Power setting */
  lcd_command(0xD0, 0x07, 0x42, 0x18);
  /* VCOM */
  lcd_command(0xD1, 0x00, 0x07, 0x10);
  /* Power setting for norm. mode */
  lcd_command(0xD2, 0x01, 0x02);
  /* Panel driving setting */
  lcd_command(0xC0, 0x10, 0x3B, 0x00, 0x02, 0x11);
  /* Frame rate & inv. */
  lcd_command(0xC5, 0x03);
  /* set_pixel_format: 16bpp */
  lcd_command(0x3A, 0x55);
  lcd_command(0x36, 0x09);
  /* @todo Implement gamma: 0xC8, 0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0C, 0x00, */
  /* set_display_on */
  lcd_command(0x29);
  lcd_set_scroll_start(UINT16_C(0));
}
