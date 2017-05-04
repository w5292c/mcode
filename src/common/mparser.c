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
#include <stdbool.h>

void mparser_parse(const char *str, size_t length, mparser_event_handler *handler)
{
  enum {
    EParsingStateInitial,
    EParsingStateToken,
    EParsingStateString,
    EParsingStateNumber,
    EParsingStateWhitespace,
  } parsingState = EParsingStateInitial;

  /* Report parsing-start */
  const char *ret = (*handler)(EParserEventBegin, str, length);
  if (ret) {
    const size_t consumed = (ret - str);
    assert(consumed > 0 && consumed <= length);
    str = ret;
    length -= consumed;
  }

  const char *ptr = str;
  while (str && length) {
    const char ch = *str;

    switch (parsingState) {
    case EParsingStateInitial:
      if ('"' == ch) {
        parsingState = EParsingStateString;
        ptr = str;
      } else if (char_is_whitespace(ch)) {
        parsingState = EParsingStateWhitespace;
        ptr = str;
      }
      break;
    case EParsingStateToken:
      break;
    case EParsingStateString:
      if ('"' == ch) {
        const size_t stringLength = (str - ptr);
        ret = (*handler)(EParserEventString, ptr, stringLength);
        if (ret) {
          str = ret;
        }
        parsingState = EParsingStateInitial;
      }
      break;
    case EParsingStateNumber:
      break;
    case EParsingStateWhitespace:
      if (!char_is_whitespace(ch)) {
        const size_t whitespaceLength = (str - ptr);
        ret = (*handler)(EParserEventSepWhitespace, ptr, whitespaceLength);
        if (ret) {
          str = ret;
        } else {
          /* Let this character be handled again */
          --str;
        }
        parsingState = EParsingStateInitial;
      }
      break;
    default:
      assert(false);
      break;
    }

    ++str;
  }

  /* Report parsing end, ingore return value */
  (*handler)(EParserEventEnd, str, length);
}
