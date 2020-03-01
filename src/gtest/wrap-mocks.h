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

#ifndef MCODE_WRAP_MOCKS_H
#define MCODE_WRAP_MOCKS_H

#include <stddef.h>
#include <gmock/gmock.h>

#ifdef __cplusplus
class MHwInterface
{
public:
  MHwInterface();
  virtual ~MHwInterface();
  virtual void uart_write_char(char ch) = 0;
  virtual void uart2_write_char(char ch) = 0;
};

class MHwMockImpl : public MHwInterface
{
public:
  MOCK_METHOD(void, uart_write_char, (char ch), (override));
  MOCK_METHOD(void, uart2_write_char, (char ch), (override));
};
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * The wrapped 'uart_write_char' function, passes request to the mock object
 */
void __wrap_uart_write_char(char ch);
void __wrap_uart2_write_char(char ch);

void collected_text_reset(void);
const char *collected_text(void);
size_t collected_text_length(void);

void collected_text2_reset(void);
const char *collected_text2(void);
size_t collected_text2_length(void);

void alt_uart_write_char(char ch);
void collected_alt_text_reset(void);
const char *collected_alt_text(void);
size_t collected_alt_text_length(void);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif /* MCODE_WRAP_MOCKS_H */
