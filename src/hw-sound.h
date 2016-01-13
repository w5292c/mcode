/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Alexander Chumakov
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

#ifndef HW_MCODE_SOUND_H
#define HW_MCODE_SOUND_H

#include <stdint.h>

void sound_init(void);
void sound_deinit(void);

/**
 * Play a note:
 * --------------------    ----------------------
 * |  # | Note | Code |    |  # | Octave | Code |
 * --------------------    ----------------------
 * |  1 |   C  | 0x00 |    |  1 |     0  | ---- |
 * |  2 |   C# | 0x01 |    |  2 |     1  | ---- |
 * |  3 |   D  | 0x02 |    |  3 |     2  | ---- |
 * |  4 |   D# | 0x03 |    |  4 |     3  | ---- |
 * |  5 |   E  | 0x04 |    |  5 |    (4) | 0x40 |
 * |  6 |   F  | 0x05 |    |  6 |     5  | 0x50 |
 * |  7 |   F# | 0x06 |    |  7 |     6  | 0x60 |
 * |  8 |   G  | 0x07 |    |  8 |     7  | 0x70 |
 * |  9 |   G# | 0x08 |    |  9 |     8  | 0x80 |
 * | 10 |  (A) | 0x09 |    ----------------------
 * | 11 |   B  | 0x0A |
 * | 12 |   H  | 0x0B |
 * --------------------
 * @param[in] note - The Note and Octave information
 * @param[in] length - The length of the note in msec
 * @note Special 'note' value 0xFFU means 'pause' for 'length' msec
 */
void sound_play_note(uint8_t note, uint16_t length);

/**
 * Play a tune stored in RAM.
 * Each item in the 'notes' has note info and time info,
 * for example: 0xTTNN; TT represents length of the note,
 * length = 20ms*TT; NN - represents the note, similar to
 * 'note' in 'sound_play_note'.
 */
void sound_play_tune(const uint16_t *notes);

/**
 * Play a tune stored in flash memory.
 * @sa sound_play_tune
 */
void sound_play_tune_P(const uint16_t *notes);

#endif /* HW_MCODE_SOUND_H */
