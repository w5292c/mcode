/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2020 Alexander Chumakov
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

#ifndef MCODE_SYSTEM_H
#define MCODE_SYSTEM_H

#include <stdint.h>

/**
 * Initialize the system
 */
void system_init(void);

/**
 * Deinitialize the system
 */
void system_deinit(void);

/*!
 * Request the system reboot
 */
void reboot(void);

/**
 * Power off the system
 */
void poweroff(void);

/*!
 * Request to launch the bootloader code
 */
void bootloader(void);

/**
 * Get the Unique Device ID
 * @param[in] index The ID index to get
 * @param The Unique Device ID
 */
uint32_t mcode_id(int index);

/**
 * Get the system frequency
 * @return The current core frequency
 */
uint32_t mcode_freq(void);

/**
 * The build-time random data
 * @return The pointer to the random data in binary format
 * @note The length of the random data is \c MCODE_RANDOM_BYTES_COUNT
 */
const uint8_t *mcode_rand(void);

#endif /* MCODE_SYSTEM_H */
