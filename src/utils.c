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

uint8_t glob_is_hex_ch (int8_t ch)
{
  /* first, check if the character is a number */
  uint8_t res = (ch <= '9' && ch >= '0');
  if (!res)
  {
    /* convert to the upper case */
    ch &= ~0x20;
    /* and check if the character is [A-F] */
    res = (ch >= 'A' && ch <= 'F');
  }
  return res;
}
