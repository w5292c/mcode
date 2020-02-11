/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2020 Alexander Chumakov
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

#ifndef MCODE_GLOBAL_H
#define MCODE_GLOBAL_H

#define __need_NULL
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#define PROGMEM
#define PSTR(value) (value)
#define strcmp_P strcmp
#define strncmp_P strncmp
#define strlen_P strlen
#define pgm_read_dword(ptr) (*((const uint32_t *)ptr))
#define pgm_read_byte(ptr) (*((const uint8_t *)ptr))
#define pgm_read_ptr_near(ptr) (ptr)
#endif /* __AVR__ */

#ifdef __cplusplus
extern "C" {
#endif

#define WEAK_API(return_type) return_type __attribute__((weak))

/**
 * Generic function template without any parameters
 */
typedef void (*mcode_tick)(void);

/**
 * Generic function to report result of an async process
 */
typedef void (*mcode_done)(bool success);

/**
 * Generic function to report result of an async read request
 */
typedef void (*mcode_read_ready)(bool success, uint8_t length, const uint8_t *data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_GLOBAL_H */
