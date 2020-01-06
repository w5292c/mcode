/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2017 Alexander Chumakov
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

#ifndef MCODE_HW_UART_H
#define MCODE_HW_UART_H

#include "mcode-config.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*hw_uart_char_event)(char aChar);
typedef void (*hw_uart_handler)(const char *data, size_t length);

void hw_uart_init(void);
void hw_uart_deinit(void);

void hw_uart_set_callback(hw_uart_char_event aCallback);

void uart_write_char(char ch);

#ifdef MCODE_UART2
/**
 * Set the callback for receiving data from UART2
 */
void hw_uart2_set_callback(hw_uart_handler cb);

/**
 * Send 'ch' to UART2
 * @param[in] ch The character to be sent
 */
void uart2_write_char(char ch);
#endif /* MCODE_UART2 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_HW_UART_H */
