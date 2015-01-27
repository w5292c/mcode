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

#ifndef MCODE_HW_LCD_H
#define MCODE_HW_LCD_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COUNT_ARGS(...)  (sizeof((int[]){__VA_ARGS__})/sizeof (int))
#define lcd_command(...) lcd_write(COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)

typedef void (*lcd_read_cb)(int length, const uint8_t *data);

void lcd_init(void);
void lcd_deinit(void);

uint16_t lcd_get_width(void);
uint16_t lcd_get_height(void);

void lcd_turn(bool on);

void lcd_set_bl(bool on);
uint32_t lcd_read_id(void);

void lcd_reset(void);
void lcd_set_read_cb(lcd_read_cb cb);

void lcd_read(uint8_t cmd, uint8_t length);
void lcd_write(int len, ...);

void lcd_set_scroll_start(uint16_t start);
void lcd_set_pages(uint16_t start, uint16_t end);
void lcd_set_columns(uint16_t start, uint16_t end);

void lcd_write_const_words(uint8_t cmd, uint16_t word, uint32_t count);

void lcd_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_HW_LCD_H */
