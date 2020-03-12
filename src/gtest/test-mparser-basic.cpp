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

#include "mparser.h"

#include "mvars.h"
#include "mglobal.h"

#include <gtest/gtest.h>

using namespace testing;

class ParserBasic : public Test
{
protected:
  void SetUp() override {
  }
  void TearDown() override {
  }
};

TEST_F(ParserBasic, StrCmpBasic)
{
  int res;

  res = mparser_strcmp("new", 3, "new");
  ASSERT_EQ(res, 0);
  res = mparser_strcmp("new", 3, "new1");
  ASSERT_NE(res, 0);
  res = mparser_strcmp("new", 3, "old");
  ASSERT_NE(res, 0);
}

TEST_F(ParserBasic, StrCmpPBasic)
{
  int res;

  res = mparser_strcmp_P("new", 3, "new");
  ASSERT_EQ(res, 0);
  res = mparser_strcmp_P("new", 3, "new1");
  ASSERT_NE(res, 0);
  res = mparser_strcmp_P("new", 3, "old");
  ASSERT_NE(res, 0);
}

TEST_F(ParserBasic, NextTokenString)
{
  uint32_t value;
  TokenType type;
  size_t length = 0;
  const char *str= NULL;
  const char *token = NULL;
  const char *const original = "\"string\"";

  str = original;
  length = strlen(original);

  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenString);
  ASSERT_EQ(value, 6);
  ASSERT_EQ(length, 0);
  ASSERT_STREQ(str, "");
  ASSERT_EQ(mparser_strcmp(token, value, "string"), 0);
}

TEST_F(ParserBasic, NextTokenNullString)
{
  TokenType type;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;

  type = next_token(NULL, &length, &token, &value);
  ASSERT_EQ(type, TokenError);
  ASSERT_EQ(value, 0);
  ASSERT_EQ(length, 0);
  ASSERT_EQ(token, MNULL);
}

TEST_F(ParserBasic, NextTokenIncorrectStringNoClosingQuote)
{
  TokenType type;
  const char *str;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "\"line";

  str = original;
  length = strlen(original);
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenError);
  ASSERT_EQ(value, 4);
  ASSERT_EQ(length, 0);
  ASSERT_EQ(token, original + 1);
  ASSERT_EQ(str, original + 5);
}

TEST_F(ParserBasic, NextTokenEmptyString)
{
  TokenType type;
  const char *str;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "";

  str = original;
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenEnd);
  ASSERT_EQ(value, 0);
  ASSERT_EQ(length, 0);
  ASSERT_EQ(token, MNULL);
}

TEST_F(ParserBasic, NextTokenNewLine)
{
  TokenType type;
  const char *str;
  size_t length = 2;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "\r\n";

  str = original;
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenNewLine);
  ASSERT_EQ(value, '\r');
  ASSERT_EQ(length, 1);
  ASSERT_EQ(token, MNULL);
  ASSERT_EQ(str, original + 1);
}

TEST_F(ParserBasic, NextTokenNumber)
{
  TokenType type;
  const char *str;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "12865535";

  str = original;
  length = strlen(original);
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenInt);
  ASSERT_EQ(value, 12865535u);
  ASSERT_EQ(length, 0);
  ASSERT_EQ(token, MNULL);
  ASSERT_EQ(str, original + 8);
}

TEST_F(ParserBasic, NextTokenNumberBeforeWhitespace)
{
  TokenType type;
  const char *str;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "127 ";

  str = original;
  length = strlen(original);
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenInt);
  ASSERT_EQ(value, 127u);
  ASSERT_EQ(length, 1);
  ASSERT_EQ(token, MNULL);
  ASSERT_EQ(str, original + 3);
}

TEST_F(ParserBasic, NextTokenInvalidNumber)
{
  TokenType type;
  const char *str;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "12865535abcd";

  str = original;
  length = strlen(original);
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenError);
  ASSERT_EQ(value, 12);
  ASSERT_EQ(length, 0);
  ASSERT_EQ(token, original);
  ASSERT_EQ(str, original + 12);
}

TEST_F(ParserBasic, NextTokenInvalidToken)
{
  TokenType type;
  const char *str;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "%hello";

  str = original;
  length = strlen(original);
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenError);
  //!@todo need to update/implement this case
}

TEST_F(ParserBasic, NextTokenVariable)
{
  TokenType type;
  const char *str;
  size_t length = 0;
  uint32_t value = 0;
  const char *token = NULL;
  const char *const original = "s3:2";

  str = original;
  length = strlen(original);
  type = next_token(&str, &length, &token, &value);
  ASSERT_EQ(type, TokenVariable);
  ASSERT_EQ(str, original + 4);
  ASSERT_EQ(length, 0);
  ASSERT_EQ(token, original);
  ASSERT_EQ(value, 4u | VarTypeString << 8 | 3 << 16 | 2 << 24);
}
