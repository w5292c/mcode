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
#include "mstring.h"
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
    io_set_ostream_handler(NULL);
  }
};

TEST_F(VarsBasic, IntVar)
{
  mvar_int_set(0, 0x12345678u);
  ASSERT_EQ(mvar_int_get(0), 0x12345678u);
}

TEST_F(VarsBasic, IntVarNonExistent)
{
  mvar_int_set(100, 0x12345678u);
  ASSERT_EQ(mvar_int_get(100), 0);
}

TEST_F(VarsBasic, NvmVar)
{
  mvar_nvm_set(0, 0x1234u);
  ASSERT_EQ(mvar_nvm_get(0), 0x1234u);
}

TEST_F(VarsBasic, NvmVarNonExistent)
{
  mvar_nvm_set(100, 0x1234u);
  ASSERT_EQ(mvar_nvm_get(100), 0);
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
  size_t length = 0;
  char *const v0 = mvar_str(0, PROG_STRVARS_COUNT, &length);
  ASSERT_NE(v0, (void *)NULL);
  ASSERT_EQ(length, PROG_STRVAR_LENGTH*PROG_STRVARS_COUNT);
}

TEST_F(VarsBasic, StringVarLargeCount)
{
  size_t length = 0;
  char *const v0 = mvar_str(2, PROG_STRVARS_COUNT, &length);
  ASSERT_NE(v0, (void *)NULL);
  ASSERT_EQ(length, PROG_STRVAR_LENGTH*PROG_STRVARS_COUNT - 2*PROG_STRVAR_LENGTH);
}

TEST_F(VarsBasic, StringVarZeroCount)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  ASSERT_NE(v0, (void *)NULL);
  ASSERT_EQ(length, 0);
}

TEST_F(VarsBasic, StringVarNonExistent)
{
  size_t length = 0;
  char *const v0 = mvar_str(100, 0, &length);
  ASSERT_EQ(v0, (void *)NULL);
  ASSERT_EQ(length, 0);
}

TEST_F(VarsBasic, StringVarPrint)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("s0:1", -1);

  ASSERT_STREQ(collected_text(), "test string");
}

TEST_F(VarsBasic, StringVarPrintNoCount)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("s0:", -1);

  ASSERT_STREQ(collected_text(), "test string");
}

TEST_F(VarsBasic, StringVarPrintOnlyIndex)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("s0", -1);

  ASSERT_STREQ(collected_text(), "test string");
}

TEST_F(VarsBasic, StringVarPrintOnlyName)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("s", -1);

  ASSERT_STREQ(collected_text(), "test string");
}

TEST_F(VarsBasic, StringVarPrintOnlyIndexWithLetter)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("s0x", -1);

  ASSERT_STREQ(collected_text(), "test string");
}

TEST_F(VarsBasic, StringVarPrintWrongIndex)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("sz", -1);

  ASSERT_STREQ(collected_text(), "");
  ASSERT_EQ(collected_text_length(), 0);
}

TEST_F(VarsBasic, StringVarPrintWrongCount)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("s0:x", -1);

  ASSERT_STREQ(collected_text(), "test string");
}

TEST_F(VarsBasic, StringVarPrintWrongVarName)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print("z0:1", -1);

  ASSERT_STREQ(collected_text(), "");
  ASSERT_EQ(collected_text_length(), 0);
}

TEST_F(VarsBasic, StringVarPrintNoVarName)
{
  size_t length = 0;
  char *const v0 = mvar_str(0, 0, &length);
  strcpy(v0, "test string");

  mvar_print(NULL, -1);

  ASSERT_STREQ(collected_text(), "");
  ASSERT_EQ(collected_text_length(), 0);
}

TEST_F(VarsBasic, IntVarPrint)
{
  mvar_int_set(0, 12345678);

  mvar_print("i0:1", -1);

  ASSERT_STREQ(collected_text(), "12345678");
}

TEST_F(VarsBasic, NvmVarPrint)
{
  mvar_nvm_set(0, 64123);

  mvar_print("n0:1", -1);

  ASSERT_STREQ(collected_text(), "64123");
}

TEST_F(VarsBasic, NvmVarPrintLong)
{
  mvar_nvm_set(0, 64123);
  mvar_nvm_set(1, 43964);

  mvar_print("n0:2", -1);

  ASSERT_STREQ(collected_text(), "2881288827");
}

TEST_F(VarsBasic, NvmVarParseVarNameNoName)
{
  size_t count;
  size_t index;
  size_t length;
  uint32_t value;
  const char *token;
  MVarType type = next_var(NULL, &length, &token, &value, &index, &count);

  ASSERT_EQ(type, VarTypeNone);
}

TEST_F(VarsBasic, NvmVarParseVarNameNullName)
{
  size_t count;
  size_t index;
  size_t length;
  uint32_t value;
  const char *token;
  const char *var_name = NULL;
  MVarType type = next_var(&var_name, &length, &token, &value, &index, &count);

  ASSERT_EQ(type, VarTypeNone);
}

TEST_F(VarsBasic, MVarPutchBasic)
{
  size_t length = 0;
  char *const buffer = mvar_str(0, 1, &length);
  memset(buffer, 0, length);

  mvar_putch_config(0, 1);
  io_set_ostream_handler(mvar_putch);

  mprintstrln("hello");
  ASSERT_STREQ(buffer, "hello\r\n");
  mprintstrln("new line of text");
  ASSERT_STREQ(buffer, "hello\r\nnew line of text\r\n");
}
