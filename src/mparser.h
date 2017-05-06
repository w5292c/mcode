/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Alexander Chumakov
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

void mparser_parse(const char *str, size_t length, mparser_event_handler *handler);

/**
 * Compare strings, first argument may not end with '\0'
 * @param[in] str The 1st string
 * @param[in] length The length of the 1st string
 * @param[in] str2 The 2nd string
 * @return 0 if 2 strings are equal, or something else if not
 */
int mparser_strcmp(const char *str, size_t length, const char *str2);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_PARSER_H */
