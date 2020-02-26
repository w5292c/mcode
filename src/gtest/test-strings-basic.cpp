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

class StringBasic : public Test
{
protected:
  void SetUp() override {
    collected_text_reset();
    collected_alt_text_reset();
  }
  void TearDown() override {
    collected_text_reset();
    collected_alt_text_reset();
  }
};

class AltStringBasic : public StringBasic
{
protected:
  void SetUp() override {
    StringBasic::SetUp();

    io_set_ostream_handler(alt_uart_write_char);
  }
  void TearDown() override {
    io_set_ostream_handler(NULL);

    StringBasic::TearDown();
  }
};

TEST_F(StringBasic, MPutchSimple)
{
  mprintstr("abc");

  ASSERT_EQ(collected_text_length(), 3);
  ASSERT_STREQ(collected_text(), "abc");
}

TEST_F(AltStringBasic, MAltPutchSimple)
{
  mprintstr("abc");

  ASSERT_EQ(collected_alt_text_length(), 3);
  ASSERT_STREQ(collected_alt_text(), "abc");
}
