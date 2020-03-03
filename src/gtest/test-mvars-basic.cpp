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

#include "mvars.h"
#include "wrap-mocks.h"

#include <gtest/gtest.h>

using namespace testing;

class VarsBasic : public Test
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

TEST_F(VarsBasic, MPutchSimple)
{
  mvar_int_set(0, 0x12345678u);
  ASSERT_EQ(mvar_int_get(0), 0x12345678u);
}

TEST_F(VarsBasic, StringVarBasic)
{
  size_t length0 = 0;
  size_t length1 = 0;
  char *const v0 = mvar_str(0, 1, &length0);
  char *const v1 = mvar_str(1, 1, &length1);
  ASSERT_EQ(length0, PROG_STRVAR_LENGTH);
  ASSERT_EQ(length1, PROG_STRVAR_LENGTH);
  ASSERT_EQ(v1, v0 + PROG_STRVAR_LENGTH);
}

TEST_F(VarsBasic, StringVarCount)
{
  char *const v0 = mvar_str(0, PROG_STRVARS_COUNT, &length0);
  
}
