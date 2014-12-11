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

#ifndef MCODE_EMU_HW_I80_H
#define MCODE_EMU_HW_I80_H

#include "hw-i80.h"

void emu_hw_i80_init (void);
void emu_hw_i80_deinit (void);

void emu_hw_i80_set_read_callback (hw_i80_read_callback aCallback);

void emu_hw_i80_read (uint8_t cmd, uint8_t length);
void emu_hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *data);

void emu_hw_i80_reset (void);

void emu_hw_i80_write_words (uint8_t cmd, uint8_t length, const uint16_t *data);
void emu_hw_i80_write_words_P (uint8_t cmd, uint8_t length, const uint16_t *data);

void emu_hw_i80_write_const_short (uint8_t cmd, uint16_t constValue, uint8_t length);
void emu_hw_i80_write_const (uint8_t cmd, uint16_t constValue, uint16_t length);
void emu_hw_i80_write_const_long (uint8_t cmd, uint16_t constValue, uint32_t length);

void emu_hw_i80_write_bitmap (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue);
void emu_hw_i80_write_bitmap_P (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue);

#endif /* MC_EMU_CODE_I80_H */
