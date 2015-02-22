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

#include <stdint.h>

void lcd_turn(bool on)
{
  if (on) {
    /* exit_sleep_mode */
    hw_i80_write(UINT8_C(0x11), 0, NULL);
    hw_i80_write(UINT8_C(0x50), 0, NULL);
    uint8_t bytes[5];
    bytes[0] = 0x07;
    bytes[1] = 0x42;
    bytes[2] = 0x18;
    /* Power setting */
    hw_i80_write(UINT8_C(0xD0), 3, bytes);
    bytes[0] = 0x00;
    bytes[1] = 0x07;
    bytes[2] = 0x10;
    /* VCOM */
    hw_i80_write(UINT8_C(0xD1), 3, bytes);
    bytes[0] = 0x01;
    bytes[1] = 0x02;
    /* Power setting for norm. mode */
    hw_i80_write(UINT8_C(0xD2), 2, bytes);
    bytes[0] = 0x10;
    bytes[1] = 0x3B;
    bytes[2] = 0x00;
    bytes[3] = 0x02;
    bytes[4] = 0x11;
    /* Panel driving setting */
    hw_i80_write(UINT8_C(0xC0), 5, bytes);
    bytes[0] = 0x03;
    /* Frame rate & inv. */
    hw_i80_write(UINT8_C(0xC5), 1, bytes);
    bytes[0] = 0x55;
    /* set_pixel_format: 16bpp */
    hw_i80_write (UINT8_C(0x3A), 1, bytes);
    bytes[0] = 0x09;
    hw_i80_write (UINT8_C(0x36), 1, bytes);
    /* @todo Implement gamma: 0xC8, 0x00, 0x32, 0x36, 0x45, 0x06, 0x16, 0x37, 0x75, 0x77, 0x54, 0x0C, 0x00, */
    /* set_display_on */
    hw_i80_write (UINT8_C(0x29), 0, NULL);
    lcd_set_scroll_start(UINT16_C(0));
  } else {
    /* set_display_off */
    hw_i80_write (UINT8_C(0x28), 0, NULL);
    /* enter_sleep_mode */
    hw_i80_write (UINT8_C(0x10), 0, NULL);
  }
}
