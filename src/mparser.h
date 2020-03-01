/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2020 Alexander Chumakov
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

#ifndef MCODE_PARSER_H
#define MCODE_PARSER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EParserEventNull,
  EParserEventBegin,
  EParserEventToken,
  EParserEventString,
  EParserEventNumber,
  EParserEventPunct,         /**< Punctuation mark, like ',', ';', ':' */
  EParserEventSepWhitespace, /**< Whitespace, like ' ' or '\t' */
  EParserEventSepEndOfLine,  /**< New lines, like '\n' or '\r' */
  EParserEventEnd,
} MParserEvent;

/**
 * Handler for parser events
 * @param[in] event The parser event ID
 * @param[in] str The (optional) pointer to the current (or next)
 *            parsing element/event
 * @param[in] length The length of the current parsing element/event
 * @param[in] value Optional value for the current parsing event
 * @return Optional pointer to the location in the parsing buffer,
 *         from which parsing should be continued
 */
typedef const char *(*mparser_event_handler)(MParserEvent event,
                                             const char *str, size_t length,
                                             int32_t value);

void mparser_parse(const char *str, size_t length, mparser_event_handler handler);

/**
 * Compare strings, first argument may not end with '\0'
 * @param[in] str The 1st string
 * @param[in] length The length of the 1st string
 * @param[in] str2 The 2nd string
 * @return 0 if 2 strings are equal, or something else if not
 */
int mparser_strcmp(const char *str, size_t length, const char *str2);

/**
 * String parser, version 2 skeleton
 */
typedef enum _TokenType {
  TokenError,
  TokenString,
  TokenId,
  TokenInt,
  TokenWhitespace,
  TokenNewLine,
  TokenCtrl,
  TokenPunct,
  TokenEnd,
} TokenType;

/**
 * Tokenize the input
 *
 * Possible tokens:
 * - String, format: "<string-content>", \c value returns the string length;
 *                   Note: no escape sequences for string tokens are supported for now;
 * - Integer: decimal number, example: 1237632, the value is returned in \c value;
 * - Whitespace: vertical and horizontal tabs, space, no new-line here;
 *               The character code is returned in \c value;
 *               Temporary: any control characters that do not fall under other types;
 * - NewLine: new line, one of the following: "\n", "\r", "\r\n";
 * - Punctuation: punctuation characters: ",", ".", ";", ":";
 * - Id: a keyword. See the list below;
 * - End: the end of string/text;
 * - Error: an error sequence of characters, for example a number combined with letters;
 *          The error token length is returned in \c value;
 * Keywords: start with a letter, can contain more letters, digits, underscores;
 * Special keywords:
 * - s<digit>[:<digit>] A string variable, up to 128 bytes per block, string blocks can be chained;
 * - i<digit> An int variable, 32 bit integer, initialized to '0' after a restart;
 * - p<digit> A 16-bit NVM int variable; keeps the stored value after a restart;
 * @note No support for Unicode for now;
 */
TokenType next_token(const char **str, size_t *length, const char **token, uint32_t *value);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_PARSER_H */
