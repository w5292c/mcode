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
char TheCollectedText2[4096];
size_t TheCollectedText2Length = 0;
char TheCollectedAltText[4096];
size_t TheCollectedAltTextLength = 0;
MHwInterface *TheMockInterface = NULL;
}

MHwInterface::MHwInterface()
{
  TheMockInterface = this;
}

MHwInterface::~MHwInterface()
{
  TheMockInterface = NULL;
}

extern "C" void __wrap_uart_write_char(char ch)
{
  if (TheMockInterface) {
    TheMockInterface->uart_write_char(ch);
    return;
  }
  TheCollectedText[TheCollectedTextLength++] = ch;
}

extern "C" void __wrap_uart2_write_char(char ch)
{
  if (TheMockInterface) {
    TheMockInterface->uart2_write_char(ch);
    return;
  }
  TheCollectedText2[TheCollectedText2Length++] = ch;
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

void collected_text2_reset(void)
{
  TheCollectedText2Length = 0;
  memset(TheCollectedText2, 0, sizeof (TheCollectedText2));
}

const char *collected_text2(void)
{
  return TheCollectedText2;
}

size_t collected_text2_length(void)
{
  return TheCollectedText2Length;
}

void alt_uart_write_char(char ch)
{
  TheCollectedAltText[TheCollectedAltTextLength++] = ch;
}

void collected_alt_text_reset(void)
{
  TheCollectedAltTextLength = 0;
  memset(TheCollectedAltText, 0, sizeof (TheCollectedAltText));
}

const char *collected_alt_text(void)
{
  return TheCollectedAltText;
}

size_t collected_alt_text_length(void)
{
  return TheCollectedAltTextLength;
}
