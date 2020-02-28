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

#include "utils.h"

#include <gtest/gtest.h>

using namespace testing;

class UtilsBasic : public Test
{
protected:
  void SetUp() override {
  }
  void TearDown() override {
  }
  bool is_whitespace(char ch) {
    return isspace(ch) || !ch;
  }
  bool is_numeric(char ch) {
    return ch >= '0' && ch <= '9';
  }
  bool is_hex(char ch) {
    return (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f') || is_numeric(ch);
  }
};

TEST_F(UtilsBasic, MCharIsWhitespace)
{
  int ch;
  for (ch = 0; ch < 256; ++ch) {
    const bool whitespace = char_is_whitespace(ch);
    ASSERT_EQ(whitespace, is_whitespace(ch)) << "Character: [" << (char)ch << "], code: " << ch;
  }
}

TEST_F(UtilsBasic, MCharIsNumeric)
{
  int ch;
  for (ch = 0; ch < 256; ++ch) {
    ASSERT_EQ(char_is_numeric(ch), is_numeric(ch)) << "Character: [" << (char)ch << "], code: " << ch;
  }
}

TEST_F(UtilsBasic, MCharIsHex)
{
  int ch;
  for (ch = 0; ch < 256; ++ch) {
    ASSERT_EQ(char_is_hex(ch), is_hex(ch)) << "Character: [" << (char)ch << "], code: " << ch;
  }
}

TEST_F(UtilsBasic, GlobChToVal)
{
  char ch;
  uint8_t value;
  for (ch = '0'; ch <= '9'; ++ch) {
    value = glob_ch_to_val(ch);
    ASSERT_EQ(value, ch - '0') << "Character: [" << (char)ch << "], code: " << ch;
  }
  for (ch = 'a'; ch <= 'f'; ++ch) {
    value = glob_ch_to_val(ch);
    ASSERT_EQ(value, ch - 'a' + 10) << "Character: [" << (char)ch << "], code: " << ch;
  }
  for (ch = 'A'; ch <= 'F'; ++ch) {
    value = glob_ch_to_val(ch);
    ASSERT_EQ(value, ch - 'A' + 10) << "Character: [" << (char)ch << "], code: " << ch;
  }
}

TEST_F(UtilsBasic, GlobGetByte)
{
  uint8_t value = glob_get_byte("9F");
  ASSERT_EQ(value, 0x9fu);
  value = glob_get_byte("00");
  ASSERT_EQ(value, 0x00u);
  value = glob_get_byte("FF");
  ASSERT_EQ(value, 0xffu);
}

TEST_F(UtilsBasic, GlobStrToUint16)
{
  uint16_t value = glob_str_to_uint16("0000");
  ASSERT_EQ(value, 0x0000u);
  value = glob_str_to_uint16("ffff");
  ASSERT_EQ(value, 0xffffu);
  value = glob_str_to_uint16("FFFF");
  ASSERT_EQ(value, 0xffffu);
  value = glob_str_to_uint16("80DE");
  ASSERT_EQ(value, 0x80deu);
  value = glob_str_to_uint16("1234");
  ASSERT_EQ(value, 0x1234u);
  value = glob_str_to_uint16("4567");
  ASSERT_EQ(value, 0x4567u);
  value = glob_str_to_uint16("89ab");
  ASSERT_EQ(value, 0x89abu);
  value = glob_str_to_uint16("cdef");
  ASSERT_EQ(value, 0xcdefu);
  value = glob_str_to_uint16("98BA");
  ASSERT_EQ(value, 0x98bau);
  value = glob_str_to_uint16("DCFE");
  ASSERT_EQ(value, 0xdcfeu);
}

TEST_F(UtilsBasic, StringSkipWhitespace)
{
  const char *const input1 = "abcdef qwerty";
  const char *const input2 = "    poiuyt asdfg";

  const char *result = string_skip_whitespace(input1);
  ASSERT_EQ(result, input1);
  result = string_skip_whitespace(result);
  ASSERT_EQ(result, input1);

  result = string_skip_whitespace(input2);
  ASSERT_EQ(result, input2 + 4);
  result = string_skip_whitespace(result);
  ASSERT_EQ(result, input2 + 4);

  result = string_skip_whitespace(NULL);
  ASSERT_EQ(result, (const char *)NULL);
  result = string_skip_whitespace("");
  ASSERT_EQ(result, (const char *)NULL);
  result = string_skip_whitespace("    ");
  ASSERT_EQ(result, (const char *)NULL);
  result = string_skip_whitespace("  \r\n\r\f\t\v ");
  ASSERT_EQ(result, (const char *)NULL);
}

TEST_F(UtilsBasic, StringNextDecimalNumber)
{
  uint16_t value = 0;
  const char *result = string_next_number("12345, new", &value);
  ASSERT_EQ(value, 12345);
  ASSERT_STREQ(result, ", new");
  result = string_next_number("12345", &value);
  ASSERT_EQ(value, 12345);
  ASSERT_EQ(result, (const char *)NULL);
  result = string_next_number("26143hello", &value);
  ASSERT_EQ(value, 26143);
  ASSERT_STREQ(result, "hello");
  result = string_next_number("1234567890 ", &value);
  ASSERT_EQ(value, 722);
  ASSERT_STREQ(result, " ");
  value = 0;
  result = string_next_number(NULL, &value);
  ASSERT_EQ(value, 0);
  ASSERT_EQ(result, (const char *)NULL);
  result = string_next_number("", &value);
  ASSERT_EQ(value, 0);
  ASSERT_EQ(result, (const char *)NULL);
}

TEST_F(UtilsBasic, StringNextHexNumber)
{
  uint16_t value = 0;
  const char *result = string_next_number("0x2345, old", &value);
  ASSERT_EQ(value, 0x2345u);
  ASSERT_STREQ(result, ", old");
  result = string_next_number("0xfe10", &value);
  ASSERT_EQ(value, 0xfe10u);
  ASSERT_EQ(result, (const char *)NULL);
  result = string_next_number("0x261fhello", &value);
  ASSERT_EQ(value, 0x261fu);
  ASSERT_STREQ(result, "hello");
  result = string_next_number("0x1234567890abcdef ", &value);
  ASSERT_EQ(value, 0xcdefu);
  ASSERT_STREQ(result, " ");
}

TEST_F(UtilsBasic, StringNextToken)
{
  int length = 0;
  const char *result = string_next_token("word 12345678", &length);
  ASSERT_EQ(length, 4);
  ASSERT_STREQ(result, " 12345678");
  result = string_next_token("token", &length);
  ASSERT_EQ(length, 5);
  ASSERT_STREQ(result, (const char *)NULL);
}

TEST_F(UtilsBasic, StringToBuffer)
{
  uint8_t length = 0;
  uint8_t buffer[128];
  memset(buffer, 0, sizeof (buffer));
  const char *result = string_to_buffer("00012ef677ff00aa", sizeof (buffer), buffer, &length);
  ASSERT_EQ(buffer[0], 0x00u);
  ASSERT_EQ(buffer[1], 0x01u);
  ASSERT_EQ(buffer[2], 0x2eu);
  ASSERT_EQ(buffer[3], 0xf6u);
  ASSERT_EQ(buffer[4], 0x77u);
  ASSERT_EQ(buffer[5], 0xffu);
  ASSERT_EQ(buffer[6], 0x00u);
  ASSERT_EQ(buffer[7], 0xaau);
  ASSERT_EQ(length, 8);
  ASSERT_EQ(result, (const char *)NULL);

  length = 0;
  memset(buffer, 0, sizeof (buffer));
  result = string_to_buffer("55X", sizeof (buffer), buffer, &length);
  ASSERT_EQ(buffer[0], 0x55u);
  ASSERT_EQ(length, 1);
  ASSERT_STREQ(result, "X");

  length = 0;
  memset(buffer, 0, sizeof (buffer));
  result = string_to_buffer("", sizeof (buffer), buffer, &length);
  ASSERT_EQ(length, 0);
  ASSERT_STREQ(result, (const char *)NULL);

  length = 0;
  memset(buffer, 0, sizeof (buffer));
  result = string_to_buffer(NULL, sizeof (buffer), buffer, &length);
  ASSERT_EQ(length, 0);
  ASSERT_STREQ(result, (const char *)NULL);

  length = 0;
  memset(buffer, 0, sizeof (buffer));
  result = string_to_buffer("1234567890abcdef", 2, buffer, &length);
  ASSERT_EQ(length, 2);
  ASSERT_EQ(buffer[0], 0x12u);
  ASSERT_EQ(buffer[1], 0x34u);
  ASSERT_STREQ(result, "567890abcdef");
}

TEST_F(UtilsBasic, DISABLED_StringToBuffer)
{
  /* This case is disabled, as it gives an error, needs to be fixed or removed */
  uint8_t length = 0;
  uint8_t buffer[128];
  memset(buffer, 0, sizeof (buffer));
  const char *result = string_to_buffer("123", sizeof (buffer), buffer, &length);
  ASSERT_EQ(length, 1);
  ASSERT_EQ(buffer[0], 0x12u);
  ASSERT_STREQ(result, "3");
}

TEST_F(UtilsBasic, FromPdu7bit)
{
  char buffer[256] = { 0 };

  size_t length = 0;
  bool res = from_pdu_7bit("F4F29C0E", -1, buffer, sizeof (buffer), &length);
  ASSERT_EQ(res, true);
  ASSERT_STREQ(buffer, "test");

  memset(buffer, 0, sizeof (buffer));
  res = from_pdu_7bit("F4F29C0EX", -1, buffer, sizeof (buffer), &length);
  /* Failure is reported, as the 'X' char is illigal in this context */
  ASSERT_EQ(res, false);
  ASSERT_STREQ(buffer, "test");

  memset(buffer, 0, sizeof  (buffer));
  res = from_pdu_7bit(NULL, -1, buffer, sizeof (buffer), &length);
  /* Failure is reported, as the 'X' char is illigal in this context */
  ASSERT_EQ(res, false);

  length = 0;
  memset(buffer, 0, sizeof (buffer));
  res = from_pdu_7bit("F3B29BDC4ABBCD6F50AC3693B14022F2DB5D16B140381A", -1, buffer, sizeof (buffer), &length);
  ASSERT_EQ(res, true);
  ASSERT_STREQ(buffer, "send-info 1532, \"done\", 84");
}
TEST_F(UtilsBasic, DISABLED_FromPdu7bit)
{
  char buffer[256] = { 0 };

  size_t length = 0;
  bool res = from_pdu_7bit("F4F29C0E", -1, buffer, 2, &length);
  /* Failure is reported, as the 'X' char is illigal in this context */
  ASSERT_EQ(res, true);
  ASSERT_EQ(length, 2);
  ASSERT_STREQ(buffer, "te");
}
