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

#ifndef MCODE_LINE_EDITOR_UART_H
#define MCODE_LINE_EDITOR_UART_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*line_editor_uart_ready) (const char *aString);

void line_editor_uart_init (void);
void line_editor_uart_deinit (void);

void line_editor_uart_set_callback (line_editor_uart_ready aCallback);

void line_editor_reset(void);
void line_editor_set_echo(bool enabled);

void line_editor_uart_start (void);
#if 0
void line_editor_uart_stop (void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_LINE_EDITOR_UART_H */
