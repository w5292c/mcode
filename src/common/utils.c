/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2020 Alexander Chumakov
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
    '\v' == ch ||
    '\f' == ch ||
    '\000' == ch);
}

bool char_is_digit(char ch)
{
  return ch >= '0' && ch <= '9';
}

bool char_is_alpha(char ch)
{
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool char_is_hex(char ch)
{
  if (char_is_digit(ch)) {
    return 1;
  } else {
    /* convert to the upper case */
    ch &= ~0x20;
    return ch >= 'A' && ch <= 'F';
  }
}

char nibble_to_char(uint8_t nibble)
{
  nibble = nibble & 0x0FU;
  if (nibble < 10) {
    return '0' + nibble;
  } else {
    return 'A' + nibble - 10;
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

const char *string_next_number(const char *str, uint16_t *value)
{
  if (!str || !*str) {
    return NULL;
  }

  if (!strncmp("0x", str, 2)) {
    return string_next_hex_number(str + 2, value);
  } else {
    return string_next_decimal_number(str, value);
  }
}

const char *string_next_decimal_number(const char *str, uint16_t *value)
{
  bool hasValue = false;
  uint16_t parsedValue = 0;
  if (str) {
    do {
      const char ch = *str;
      if (!ch) {
        str = NULL;
        break;
      } else if (!char_is_digit(ch)) {
        break;
      } else {
        hasValue = true;
        const uint8_t charValue = glob_ch_to_val(ch);
        parsedValue *= 10u;
        parsedValue += charValue;
        ++str;
      }
    } while (1);
  }

  if (hasValue && value) {
    *value = parsedValue;
  }

  return str;
}

const char *string_next_hex_number(const char *str, uint16_t *value)
{
  bool hasValue = false;
  uint16_t parsedValue = 0;
  if (str) {
    do {
      const char ch = *str;
      if (!ch) {
        str = NULL;
        break;
      } else if (!char_is_hex(ch)) {
        break;
      } else {
        hasValue = true;
        const uint8_t charValue = glob_ch_to_val(ch);
        parsedValue *= 16;
        parsedValue += charValue;
        ++str;
      }
    } while (1);
  }

  if (hasValue && value) {
    *value = parsedValue;
  }

  return str;
}

const char *string_next_token(const char *str, int *length)
{
  int result = 0;
  if (str) {
    do {
      const char ch = *str;
      if (!ch) {
        str = NULL;
        break;
      } else if (char_is_whitespace(ch)) {
        break;
      } else {
        ++str;
        ++result;
      }
    } while (1);
  }

  if (length) {
    *length = result;
  }

  return str;
}

const char *string_to_buffer(const char *str,
                             uint8_t bufferLength, uint8_t *buffer, uint8_t *bufferFilled)
{
  if (!str || !*str) {
    return NULL;
  }

  uint8_t filled = 0;
  bool intial = true;
  uint8_t data = 0;
  do {
    /* first, check if we have enough space in the output buffer */
    if (filled + 1 > bufferLength) {
      /* no more space in the output buffer, stop parsing */
      break;
    }

    const uint8_t ch = *str;
    if (!ch) {
      /* no more chars, exit */
      str = NULL;
      break;
    }
    if (!char_is_hex(ch)) {
      /* stop parsing, as the current char is not a hex digit */
      break;
    }

    data = (data << 4);
    data |= glob_ch_to_val(ch);
    if (intial) {
      intial = false;
    } else {
      /* 'data' has been prepared, put it to the output buffer */
      buffer[filled] = data;
      ++filled;
      data = 0;
      intial = true;
    }

    ++str;
  } while (true);

  if (bufferFilled) {
    *bufferFilled = filled;
  }

  return str;
}

#ifdef MCODE_PDU
bool from_pdu_7bit(const char *pdu, size_t pduLength, char *out, size_t outMaxLength, size_t *outLength)
{
  if (!pdu || !out || !outLength || !outMaxLength) {
    /* Wrong arguments */
    return false;
  }

  if (-1 == pduLength) {
    /* Update the length, if input pduLength is not defined */
    pduLength = strlen(pdu);
  }

  char ch;
  uint16_t temp = 0;
  bool result = true;
  uint8_t currentByte;
  size_t inBufferIndex;
  size_t outBufferIndex;
  for (inBufferIndex = 0, outBufferIndex = 0; pduLength; ++pdu, --pduLength) {
    ch = *pdu;
    if (!char_is_hex(ch)) {
      /* Detected invalid character, finish processing */
      result = false;
      break;
    }

    const uint8_t current = glob_ch_to_val(ch);
    currentByte = currentByte << 4;
    currentByte = currentByte | current;
    ++inBufferIndex;
    if (inBufferIndex & 1) {
      /* We are in the middle of parsing another byte, continue */
      continue;
    }

    /* For each 2nd item we get the whole byte of input data */
    temp = temp >> 8;
    temp = temp | ((uint16_t)currentByte << 8);

    /* Check if the output buffer has enough space for another character */
    if (outBufferIndex + 1 >= outMaxLength) {
      /* We reached the end of output buffer, convertion done */
      break;
    }

    switch (outBufferIndex % 8) {
    case 0:
      out[outBufferIndex++] = ((temp >> 8) & 0x7FU);
      break;
    case 1:
      out[outBufferIndex++] = ((temp >> 7) & 0x7FU);
      break;
    case 2:
      out[outBufferIndex++] = ((temp >> 6) & 0x7FU);
      break;
    case 3:
      out[outBufferIndex++] = ((temp >> 5) & 0x7FU);
      break;
    case 4:
      out[outBufferIndex++] = ((temp >> 4) & 0x7FU);
      break;
    case 5:
      out[outBufferIndex++] = ((temp >> 3) & 0x7FU);
      break;
    case 6:
      out[outBufferIndex++] = ((temp >> 2) & 0x7FU);
      if (outBufferIndex + 1 < outMaxLength) {
        out[outBufferIndex++] = ((temp >> 9) & 0x7FU);
      }
      break;
    case 7:
    default:
      /* We must not appear here */
      break;
    }
  }

  return result;
}
#endif /* MCODE_PDU */
