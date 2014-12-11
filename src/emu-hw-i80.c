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

#include "emu-hw-i80.h"

#include "hw-uart.h"
#include "mcode-config.h"

#ifdef MCODE_EMULATE_I80_LCD1
#include "emu-hw-i80-lcd1.h"
#elif defined (MCODE_EMULATE_LCD_S95513)
#include "emu-hw-lcd-s95513.h"
#endif

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

#ifndef MCODE_EMULATE_I80
#error "The file 'emu-hw-i80.c' can only be included in MCODE_EMULATE_I80 mode"
#endif /* MCODE_EMULATE_I80 */

void emu_hw_i80_init (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_init ();
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_init ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_init: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_deinit (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_deinit ();
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_deinit ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_deinit: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_set_read_callback (aCallback);
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_set_read_callback (aCallback);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_set_read_callback ("));
  hw_uart_write_uint (((uint64_t)(aCallback)));
  hw_uart_write_string_P (PSTR("): no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *data)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_write (cmd, length, data);
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write (cmd, length, data);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_read (uint8_t cmd, uint8_t length)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_read (cmd, length);
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_read (cmd, length);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_read: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_reset (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_reset ();
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_reset ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_reset: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write_words (uint8_t cmd, uint8_t length, const uint16_t *data)
{
  #ifdef MCODE_EMULATE_I80_LCD1
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write_words (cmd, length, data);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write_words: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write_words_P (uint8_t cmd, uint8_t length, const uint16_t *data)
{
#ifdef MCODE_EMULATE_I80_LCD1
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write_words_P (cmd, length, data);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write_words_P: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write_const_short (uint8_t cmd, uint16_t constValue, uint8_t length)
{
#ifdef MCODE_EMULATE_I80_LCD1
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write_const_short (cmd, constValue, length);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write_const_short: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write_const (uint8_t cmd, uint16_t constValue, uint16_t length)
{
#ifdef MCODE_EMULATE_I80_LCD1
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write_const (cmd, constValue, length);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write_const: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write_const_long (uint8_t cmd, uint16_t constValue, uint32_t length)
{
#ifdef MCODE_EMULATE_I80_LCD1
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write_const_long (cmd, constValue, length);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write_const_long: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write_bitmap (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
#ifdef MCODE_EMULATE_I80_LCD1
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write_bitmap (cmd, length, pData, offValue, onValue);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write_bitmap: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write_bitmap_P (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
#ifdef MCODE_EMULATE_I80_LCD1
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write_bitmap_P (cmd, length, pData, offValue, onValue);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write_bitmap_P: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}
