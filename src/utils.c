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

#include "utils.h"

#include <string.h>

uint16_t glob_str_to_uint16(const char *pHexString)
{
  return glob_get_byte(pHexString + 2) | (((uint16_t)glob_get_byte(pHexString)) << 8);
}

uint8_t glob_get_byte(const char *pData)
{
  return (glob_ch_to_val (pData[0]) << 4) | glob_ch_to_val (pData[1]);
}

uint8_t glob_ch_to_val(uint8_t ch)
{
  uint8_t value = ch;

  /* 'A' (10): 0x41, 0x61 */
  if (value > '9')
  {
    value = (value & (~0x20));
    value += 10 - 'A';
  }
  else
  {
    value -= '0';
  }

  return value;
}

bool char_is_whitespace(char ch)
{
  return
    (' ' == ch ||
    '\r' == ch ||
    '\n' == ch ||
    '\t' == ch ||
    '\000' == ch);
}

bool char_is_numeric(char ch)
{
  return ch >= '0' && ch <= '9';
}

bool char_is_hex(char ch)
{
  if (char_is_numeric(ch)) {
    return 1;
  } else {
    /* convert to the upper case */
    ch &= ~0x20;
    return ch >= 'A' && ch <= 'F';
  }
}

const char *string_skip_whitespace(const char *str)
{
  if (str) {
    do {
      const char ch = *str;
      if (!ch) {
        /* end-of-line reached */
        str = 0;
        break;
      } else if (char_is_whitespace(ch)) {
        /* whitespace, skip it */
        ++str;
      } else {
        /* non-whitespace found, return it */
        break;
      }
    } while (1);
  }

  return str;
}

const char *string_next_number(const char *str, int *value)
{
  if (!strncmp("0x", str, 2)) {
    return string_next_hex_number(str + 2, value);
  } else {
    return string_next_decimal_number(str, value);
  }
}

const char *string_next_decimal_number(const char *str, int *value)
{
  int parsedValue = 0;
  if (str) {
    do {
      const char ch = *str;
      if (!ch || !char_is_numeric(ch)) {
        break;
      } else {
        const uint8_t charValue = glob_ch_to_val(ch);
        parsedValue *= 10;
        parsedValue += charValue;
        ++str;
      }
    } while (1);
  }

  *value = parsedValue;

  return str;
}

const char *string_next_hex_number(const char *str, int *value)
{
  int parsedValue = 0;
  if (str) {
    do {
      const char ch = *str;
      if (!ch || !char_is_hex(ch)) {
        break;
      } else {
        const uint8_t charValue = glob_ch_to_val(ch);
        parsedValue *= 16;
        parsedValue += charValue;
        ++str;
      }
    } while (1);
  }

  *value = parsedValue;

  return str;
}

const char *string_next_token(const char *str, int *length)
{
  int result = 0;
  if (str) {
    do {
      const char ch = *str;
      if (!ch) {
        str = 0;
        break;
      } else if (char_is_whitespace(ch)) {
        break;
      } else {
        ++str;
        ++result;
      }
    } while (1);
  }

  *length = result;

  return str;
}
