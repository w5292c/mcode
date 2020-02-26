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

#include "mstring.h"
#include "wrap-mocks.h"

#include <gtest/gtest.h>

using namespace testing;

extern "C" void ostream_handler_impl(char ch);

class StringTest : public Test
{
protected:
  void SetUp() override {
  }
  void TearDown() override {
  }

protected:
  StrictMock<MHwMockImpl> _mock;
};

TEST_F(StringTest, MPutchSimple)
{
  InSequence sequence;
  EXPECT_CALL(_mock, uart_write_char('a'))
    .Times(1);
  EXPECT_CALL(_mock, uart_write_char('b'))
    .Times(1);
  EXPECT_CALL(_mock, uart_write_char('c'))
    .Times(1);

  mprintstr("abc");
}

TEST(StringBasic, MPutchSimple)
{
  collected_text_reset();

  mprintstr("abc");

  ASSERT_EQ(collected_text_length(), 3);
  ASSERT_STREQ(collected_text(), "abc");
}

TEST(StringBasic, MAltPutchSimple)
{
  collected_alt_text_reset();
  io_set_ostream_handler(alt_uart_write_char);

  mprintstr("abc");

  ASSERT_EQ(collected_alt_text_length(), 3);
  ASSERT_STREQ(collected_alt_text(), "abc");
  io_set_ostream_handler(NULL);
}

void ostream_handler_impl(char ch)
{
}
