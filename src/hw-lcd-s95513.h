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

#ifndef MCODE_HW_LCD_S95513_H
#define MCODE_HW_LCD_S95513_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hw_lcd_s95513_turn_on (void);
void hw_lcd_s95513_turn_off (void);
void hw_lcd_s95513_set_scroll_start (uint16_t start);

#define LCD_S95513_WR_RAM_START UINT8_C(0x2C)
#define LCD_S95513_WR_RAM_CONT UINT8_C(0x3C)
#define LCD_S95513_SET_COLUMN_ADDR UINT8_C(0x2A)
#define LCD_S95513_SET_PAGE_ADDR UINT8_C(0x2B)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_HW_LCD_S95513_H */
