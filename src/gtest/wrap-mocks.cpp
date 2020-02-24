/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Alexander Chumakov
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

#include <gtest/gtest.h>

#include "wrap-mocks.h"

namespace {
char TheCollectedText[4096];
size_t TheCollectedTextLength = 0;
}

extern "C" void __wrap_uart_write_char(char ch)
{
  TheCollectedText[TheCollectedTextLength++] = ch;
}

const char *collected_text(void)
{
  return TheCollectedText;
}

size_t collected_text_length(void)
{
  return TheCollectedTextLength;
}

void collected_text_reset(void)
{
  TheCollectedTextLength = 0;
  memset(TheCollectedText, 0, sizeof (TheCollectedText));
}
