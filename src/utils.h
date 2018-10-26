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

#ifndef MCODE_UTILS_H
#define MCODE_UTILS_H

#include "mcode-config.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t glob_ch_to_val(uint8_t ch);
uint8_t glob_get_byte(const char *pData);
uint16_t glob_str_to_uint16(const char *pHexString);

bool char_is_whitespace(char ch);
bool char_is_numeric(char ch);
bool char_is_hex(char ch);

char nibble_to_char(uint8_t nibble);

const char *string_skip_whitespace(const char *str);
const char *string_next_number(const char *str, uint16_t *value);
/**
 * Extract the next token in the input string
 * @param[in] str The input string to parse
 * @param[in] length The optional size of the input string, or -1 if the string is '\0'-terminated
 * @return The pointer to the next character after the detected token, or NULL if end-of-line reached
 */
const char *string_next_token(const char *str, size_t length);
const char *string_next_hex_number(const char *str, uint16_t *value);
const char *string_next_decimal_number(const char *str, uint16_t *value);

const char *string_to_buffer(const char *str,
                             uint8_t bufferLength, uint8_t *buffer, uint8_t *bufferFilled);

#ifdef MCODE_PDU
bool from_pdu_7bit(const char *pdu, size_t pduLength, char *out, size_t outMaxLength, size_t *outLength);
#endif /* MCODE_PDU */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_UTILS_H */
