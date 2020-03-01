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

#ifndef MCODE_STRINGS_H
#define MCODE_STRINGS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  MStringNull,
  MStringNewLine,
  MStringError,
  MStringWarning,
  MStringInternalError,
  MStringWrongArgument,
  MStringWrongMode,
  MStringErrorLimit,
  MStringEnterPass,
} MCodeStringId;

/**
 * This type defines prototype for a output stream handler
 * implementing actual output of strings to output devices.
 */
typedef void (*ostream_handler)(char ch);

/**
 * Set output stream handler
 * @param[in] handler The new output stream handler, or \c NULL to reset to default handler
 * @note The default handler sends the output stream to UART1 device (can be overridden)
 */
void io_set_ostream_handler(ostream_handler handler);

const char *mstring(uint8_t id);

void mputch(char ch);

void merror(uint8_t id);
void mwarning(uint8_t id);
void mprint(uint8_t id);
void mprintln(uint8_t id);
void mprintstr(const char *string);
void mprintstrln(const char *string);
void mprintbytes(const char *str, size_t length);
void mprintbytesln(const char *str, size_t length);
void mprinthexencodedstr16(const char *str, size_t length);

void mprintstr_R(const char *string);

/**
 * Print expression in \c exprt to the default terminal
 * @param[in] expr The string with possible escape sequences and variables
 */
void mprintexpr(const char *expr);

void mprint_uint8(uint8_t value, bool skipZeros);
void mprint_uint16(uint16_t value, bool skipZeros);
void mprint_uint32(uint32_t value, bool skipZeros);
void mprint_uint64(uint64_t value, bool skipZeros);
void mprint_uintd(uint32_t value, uint8_t minDigits);

void mprint_dump_buffer(uint8_t length, const void *data, bool showAddress);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_STRINGS_H */
