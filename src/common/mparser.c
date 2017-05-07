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

#include "mparser.h"

#include "utils.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>

static inline bool mparser_is_punct(char ch);
static inline bool mparser_is_number(char ch);
static inline bool mparser_is_whitespace(char ch);

void mparser_parse(const char *str, size_t length, mparser_event_handler handler)
{
  enum {
    EParsingStateInitial,
    EParsingStateToken,
    EParsingStateString,
    EParsingStateNumber,
    EParsingStateWhitespace,
  } parsingState = EParsingStateInitial;

  /* Report parsing-start */
  const char *ret = (*handler)(EParserEventBegin, str, length, 0);
  if (ret) {
    const size_t consumed = (ret - str);
    assert(consumed > 0 && consumed <= length);
    str = ret;
    length -= consumed;
  }

  int32_t number = 0;
  const char *ptr = str;
  while (str && length) {
    const char ch = *str;
    if (!ch) {
      break;
    }

    switch (parsingState) {
    case EParsingStateInitial:
      if ('"' == ch) {
        parsingState = EParsingStateString;
        ptr = ++str; --length;
      } else if (mparser_is_whitespace(ch)) {
        parsingState = EParsingStateWhitespace;
        ptr = str++;
      } else if (mparser_is_punct(ch)) {
        /* Ignore the return value for now */
        /** @todo check if need to handle the return value here */
        (*handler)(EParserEventPunct, str, 1, ch);
        ++str; --length;
      } else if (mparser_is_number(ch)) {
        number = ch - '0';
        parsingState = EParsingStateNumber;
        ptr = str++;
      } else if ('\n' == ch || '\r' == ch) {
        ret = (*handler)(EParserEventSepEndOfLine, str + 1, length - 1, ch);
        if (ret) {
          const size_t ln = (ret - str);
          str = ret;
          length -= ln;
        } else {
          ++str; --length;
        }
      } else {
        parsingState = EParsingStateToken;
        ptr = str++;
      }
      break;
    case EParsingStateToken:
      if (mparser_is_whitespace(ch) ||
          mparser_is_punct(ch) ||
          '\n' == ch || '\r' == ch) {
        /* End of token detected */
        const size_t ln = (str - ptr);
        /* Ignore the return value for now */
        /** @todo Check if return value should be handled here */
        (*handler)(EParserEventToken, ptr, ln, 0);
        length -= ln;
        parsingState = EParsingStateInitial;
      } else {
        ++str;
      }
      break;
    case EParsingStateString:
      if ('"' == ch) {
        const size_t ln = (str - ptr);
        (*handler)(EParserEventString, ptr, ln, 0);
        length -= ln + 1;
        parsingState = EParsingStateInitial;
        ++str;
      } else {
        ++str;
      }
      break;
    case EParsingStateNumber:
      if (mparser_is_number(ch)) {
        number *= 10;
        number += ch - '0';
        ++str;
      } else {
        const size_t ln = (str - ptr);
        (*handler)(EParserEventNumber, ptr, ln, number);
        length -= ln;
        parsingState = EParsingStateInitial;
      }
      break;
    case EParsingStateWhitespace:
      if (mparser_is_whitespace(ch)) {
        ++str;
      } else {
        const size_t ln = (str - ptr);
        ret = (*handler)(EParserEventSepWhitespace, ptr, ln, 0);
        if (ret) {
          str = ret;
        } else {
          length -= ln;
        }
        parsingState = EParsingStateInitial;
      }
      break;
    default:
      assert(false);
      break;
    }
  }

  /* Report parsing end, ingore return value */
  (*handler)(EParserEventEnd, str, length, 0);
}

int mparser_strcmp(const char *str, size_t length, const char *str2)
{
  const size_t length2 = strlen(str2);
  if (length == length2) {
    return memcmp(str, str2, length);
  } else {
    /* No more/less for now */
    return 1;
  }
}

bool mparser_is_punct(char ch)
{
  return ',' == ch || ';' == ch || ':' == ch;
}

bool mparser_is_number(char ch)
{
  return '0' <= ch && '9' >= ch;
}

bool mparser_is_whitespace(char ch)
{
  return ' ' == ch || '\t' == ch;
}
