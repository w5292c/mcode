/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Alexander Chumakov
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

#include "gsm-utils.h"

#include <string.h>

uint8_t *gsm_utils_encode_phonenumber(const char *phone, uint8_t *encoded, size_t length)
{
  char ch;
  uint8_t *stored = encoded;

  // "+27831000015" => 0x"72 38 01 00 10 F5";
  //                      72 38 01 00 10 05
  if (!phone || !encoded || !length) {
    /* Wrong argument */
    return 0;
  }

  ch = *phone;
  if ('+' == ch) {
    ch = *(++phone);
  }

  memset(encoded, 0, length);
  bool closing = false;
  while (ch && length) {
    /* Parse the character */
    if (ch < '0' || ch > '9') {
      break;
    }

    /* */
    ch -= '0';
    *encoded |= (ch << (closing ? 4 : 0));

    /* Move on to the next character */
    closing = !closing;
    if (!closing) {
      ++encoded;
      --length;
    }

    ch = *(++phone);
  }
  /* The the next character is closing but it does not exist, update it to '0xF' */
  if (closing) {
    *encoded |= 0xf0u;
    ++encoded;
  }

  return encoded;
}
