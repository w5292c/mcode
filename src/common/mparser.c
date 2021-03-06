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

#include "mparser.h"

#include "mvars.h"
#include "utils.h"
#include "mglobal.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>

static inline bool mparser_is_punct(char ch);
static inline bool mparser_is_whitespace(char ch);

#ifdef MCODE_OLD_PARSER
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
      } else if (char_is_digit(ch)) {
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
      if (char_is_digit(ch)) {
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
          /// @todo check length
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
#endif /* MCODE_OLD_PARSER */

int mparser_strcmp(const char *str, size_t length, const char *str2)
{
  const size_t length2 = strlen(str2);
  if (length == length2) {
    return strncmp(str, str2, length);
  } else {
    /* No more/less for now */
    return 1;
  }
}

int mparser_strcmp_P(const char *str, size_t length, const char *str2)
{
  const size_t length2 = strlen_P(str2);
  if (length == length2) {
    return strncmp_P(str, str2, length);
  } else {
    /* No more/less for now */
    return 1;
  }
}

bool mparser_is_punct(char ch)
{
  return ',' == ch || ';' == ch || ':' == ch || '+' == ch;
}

bool mparser_is_whitespace(char ch)
{
  return ' ' == ch || '\t' == ch || '\v' == ch;
}

TokenType next_token(const char **str, size_t *length, const char **token, uint32_t *value)
{
  char ch;
  size_t len;
  uint32_t val;
  const char *ptr;
  const char *addr;

  /* Check arguments */
  if (!str || !length || !value || !*str) {
    return TokenError;
  }

  /* Check for end-of-string */
  ptr = *str;
  ch = *ptr;
  len = *length;
  /* No support for unicode for now */
  if (!len || !ch || ch < 0) {
    return TokenEnd;
  }

  /* Check punctuation characters */
  if (mparser_is_punct(ch)) {
    *value = ch;
    *str = ++ptr;
    *length = len - 1;
    return TokenPunct;
  }

  /* Check for whitespace characters */
  if (mparser_is_whitespace(ch)) {
    *value = ch;
    *str = ++ptr;
    *length = len - 1;
    return TokenWhitespace;
  }

  /* Check the character for new-line */
  if ('\n' == ch || '\r' == ch) {
    *value = ch;
    *str = ++ptr;
    *length = len - 1;
    return TokenNewLine;
  }

  /* Check for a string token */
  if ('"' == ch) {
    /* Store the pointer to the beginning of the string */
    addr = ptr + 1;
    /* Search for the closing '"' */
    for (++ptr; len > 0; --len, ++ptr) {
      const char next_ch = *ptr;
      if ('"' == next_ch) {
        /* Closing '"' detected, report a string token */
        *value = ptr - *str - 1;
        *length -= *value + 2;
        *token = addr;
        *str = ptr + 1;
        return TokenString;
      }
      if (next_ch <= 0 || '\n' == next_ch || '\r' == next_ch) {
        /* No closing '"' found, report an error */
        *value = ptr - *str - 1;
        *length -= *value + 1;
        *token = addr;
        *str = ptr;
        return TokenError;
      }
    }
  }

  /* Check for a number token */
  if (char_is_digit(ch)) {
    addr = ptr;
    val = ch - '0';
    /* Search for the closing the current number */
    for (++ptr, --len; len > 0; --len, ++ptr) {
      const char next_ch = *ptr;
      if (char_is_digit(next_ch)) {
        val *= 10;
        val += next_ch - '0';
      } else if (mparser_is_punct(next_ch) || mparser_is_whitespace(next_ch) ||
                                  '\n' == next_ch || '\r' == next_ch || !next_ch) {
        /* Expected closing of a number */
        *value = val;
        *length -= ptr - addr;
        *str = ptr;
        return TokenInt;
      } else {
        /* Unexpected characters in a number, report an error */
        for (; len > 0; --len, ++ptr) {
          ch = *ptr;
          if (mparser_is_whitespace(ch) || mparser_is_punct(ch) ||
              '\n' == ch || '\r' == ch || !ch) {
            break;
          }
        }
        *value = ptr - *str;
        *length -= *value;
        *str = ptr;
        *token = addr;
        return TokenError;
      }
    }

    /* Looks like a number in the end of the input text */
    *value = val;
    *length -= ptr - addr;
    *str = ptr;
    return TokenInt;
  }

  /* Check for an ID token */
  if (char_is_alpha(ch)) {
    /* Store the beginning of the token */
    addr = ptr;
    for (++ptr; len > 0; --len, ++ptr) {
      const char next_ch = *ptr;
      if ((mparser_is_whitespace(next_ch) ||
           '\n' == next_ch || '\r' == next_ch || 0 == next_ch ||
           mparser_is_punct(next_ch)) &&
           /* Exceptions are listed here, for example ':' does not break IDs */
           (next_ch != ':')) {
        size_t index;
        size_t count;
        MVarType type;

        /* End of token detected */
        *value = ptr - *str;
        *length -= *value;
        *token = addr;
        *str = ptr;

        /* Finally, check if the current token is a variable name */
        type = var_parse_name(addr, *value, &index, &count);
        if (VarTypeNone != type) {
          *value = *value | (((uint32_t)type & 0xffu) << 8) |
                   (((uint32_t)index & 0xffu) << 16) | (((uint32_t)count & 0xffu) << 24);
          return TokenVariable;
        } else {
          return TokenId;
        }
      }
    }
  }

  return TokenError;
}
