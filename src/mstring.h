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

#define MCODE_OUTPUT_STACK_COUNT (4)

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
 * @note Use this function to temporary update the output handler in a single function call,
 *       restore it to \c NULL at exit from any function.
 * @note Restoring to \c NULL might be implemented in after return from scheduler handlers
 */
void io_set_ostream_handler(ostream_handler handler);

/**
 * Push a custom output handler to the stack of output handlers
 * @param[in] handler The output handler to use for character output in \c mputch
 * @note This function should have the matching \c io_ostream_handler_pop to restore
 *       the default output hander
 * @note The handlers in the output stack will be used if the handler is not set uisng
 *       \c io_set_ostream_handler function, if it is set, it will be used instead
 */
void io_ostream_handler_push(ostream_handler handler);

/**
 * Pop the last previously pushed output handler activating the previous handler
 * @note If there are no other output handlers in the stack, default handler will be activated.
 * @note There are \c MCODE_OUTPUT_STACK_COUNT items exist in the stack
 */
void io_ostream_handler_pop(void);

/**
 * Return the string corresponding to the string ID passed in \c id
 * @param[in] id The string ID to be returned
 * @return The pointer to the string corresponding to the string ID passed in \c id
 * @note If supported on a specific platform, the string will be stored in flash memory
 */
const char *mstring(uint8_t id);

/**
 * This is the single point to implement actual output a character in \c ch to UART1 device
 * @param[in] ch The character to be sent to UART1 device for output
 * @note This should be the only output point to UART1 device
 * @note This is the default handler for output data
 */
void mputch(char ch);

/**
 * Put character handler for appending the character to the previously configured string buffer
 * @param[in] The character to append to the previously configured string buffer
 */
void mputch_str(char ch);

/**
 * Confure the string buffer for \c mputch_str handler
 * @param[in] buffer The pointer to the start of the string buffer to be used with \c mputch_str
 * @param[in] length The length of the string buffer to be used with \c mputch_str
 */
void mputch_str_config(char *buffer, size_t length);

void merror(uint8_t id);
void mwarning(uint8_t id);
void mprint(uint8_t id);
void mprintln(uint8_t id);
void mprintstr(const char *string);
void mprintstrln(const char *string);
void mprintbytes(const char *str, size_t length);
void mprintbytesln(const char *str, size_t length);
/**
 * Encode the input passed in \str in HEX16 encoding and print result
 * @param[in] str The input string to be HEX16-encoded and printed
 * @param[in] length The length of the input string to be encoded/printed
 */
void mprintstrhex16encoded(const char *str, size_t length);
/**
 * Decode HEX16-encoded string and print result
 * @param[in] str The HEX16 encoded string to decode and print
 * @param[in] length The length of the input string or \c -1 to use the whole string
 * @note Only ASCII encoded characters are supported
 */
void mprinthexencodedstr16(const char *str, size_t length);
/**
 * Print the binary input data in hex8 encoding
 * @param[in] data The input data to print
 * @param[in] length The length of the data or \c -1 if input data is a \c null-terminated string
 */
void mprinthexencodeddata8(const void *data, size_t length);

void mprintstr_R(const char *string);
void mprintbytes_R(const char *str, size_t length);

/**
 * Print the expression in RAM pointed by \c str to the default terminal
 * @param[in] str The string with possible escape sequences and variables
 * @param[in] length The length of the string or may be \c -1 if it is NULL-terminated
 */
void mprintexpr(const char *str, size_t length);

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
