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

TEST_F(StringTest, MPrintStrSimple)
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

TEST_F(StringTest, MPrintStrLnSimple)
{
  InSequence sequence;
  EXPECT_CALL(_mock, uart_write_char('d')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('e')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('f')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('\r')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('\n')).Times(1);

  mprintstrln("def");
}

TEST_F(StringTest, MPrintBytesSimple)
{
  InSequence sequence;
  EXPECT_CALL(_mock, uart_write_char('q')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('w')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('e')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('r')).Times(1);

  mprintbytes("qwer", -1);
}

TEST_F(StringTest, MPrintBytesLnSimple)
{
  InSequence sequence;
  EXPECT_CALL(_mock, uart_write_char('p')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('o')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('i')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('u')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('\r')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('\n')).Times(1);

  mprintbytesln("poiu", -1);
}

TEST_F(StringTest, MPrintBytesWithLength)
{
  InSequence sequence;
  EXPECT_CALL(_mock, uart_write_char('q')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('w')).Times(1);

  mprintbytes("qwer", 2);
}

TEST_F(StringTest, MPrintBytesLnWithLength)
{
  InSequence sequence;
  EXPECT_CALL(_mock, uart_write_char('p')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('o')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('\r')).Times(1);
  EXPECT_CALL(_mock, uart_write_char('\n')).Times(1);

  mprintbytesln("poiu", 2);
}
