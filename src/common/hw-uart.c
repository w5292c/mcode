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

#include "hw-uart.h"

#include "utils.h"

#include <string.h>

void hw_uart_deinit(void)
{
}

void hw_uart_start_read(void)
{
}

void hw_uart_write_uintd(uint32_t value, bool skipZeroes)
{
  char buffer[2] = {0, 0};
  uint32_t temp;
  uint32_t factor = 1000000000U;
  while (factor) {
    temp = value/factor;
    if (temp || !skipZeroes) {
      buffer[0] = temp + '0';
      hw_uart_write_string(buffer);
    }
    if (temp) {
      skipZeroes = false;
      value -= factor*temp;
    }

    factor /= 10;
  }
}

void hw_uart_write_uint(uint16_t value)
{
  hw_uart_write_uint16(value, false);
}

void hw_uart_write_uint64(uint64_t value, bool skipZeros)
{
  const uint32_t upper = (uint32_t)(value>>32);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    hw_uart_write_uint32(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  hw_uart_write_uint32((uint32_t)value, skipZeros);
}

void hw_uart_write_uint32(uint32_t value, bool skipZeros)
{
  const uint16_t upper = (uint16_t)(value>>16);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    hw_uart_write_uint16(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  hw_uart_write_uint16((uint16_t)value, skipZeros);
}

void hw_uart_write_uint16(uint16_t value, bool skipZeros)
{
  int i;
  char buffer[5];
  buffer[0] = nibble_to_char(0x0FU & (value >> 12));
  buffer[1] = nibble_to_char(0x0FU & (value >>  8));
  buffer[2] = nibble_to_char(0x0FU & (value >>  4));
  buffer[3] = nibble_to_char(0x0FU & value);
  buffer[4] = 0;
  if (skipZeros) {
    for (i = 0; i < 3; ++i) {
      if ('0' == *buffer) {
        memmove(buffer, buffer + 1, 4);
      }
    }
  }
  hw_uart_write_string(buffer);
}

void hw_uart_write_string(const char *aString)
{
  uint8_t ch;
  while (0 != (ch = *aString++)) {
    uart_write_char(ch);
  }
}
