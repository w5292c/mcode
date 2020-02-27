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

class StringWithIds : public TestWithParam<int>
{
protected:
  void SetUp() override {
    TestWithParam<int>::SetUp();

    collected_text_reset();
    collected_alt_text_reset();
  }
  void TearDown() override {
    collected_text_reset();
    collected_alt_text_reset();

    TestWithParam<int>::TearDown();
  }

  const char *ExpectedBaseString() const {
    switch (GetParam()) {
    case MStringNull:
      return "";
    case MStringNewLine:
      return "\r\n";
    case MStringError:
      return "Error: ";
    case MStringWarning:
      return "Warning: ";
    case MStringInternalError:
      return "internal error";
    case MStringWrongArgument:
      return "wrong argument(s)";
    case MStringWrongMode:
      return "only root can do this";
    case MStringErrorLimit:
      return "limit reached";
    case MStringEnterPass:
      return "Enter password: ";
    default:
      return "##undefined##";
    }
  }
};

INSTANTIATE_TEST_CASE_P(
  StringIds,
  StringWithIds,
  Values(
    MStringNull,
    MStringNewLine,
    MStringError,
    MStringWarning,
    MStringInternalError,
    MStringWrongArgument,
    MStringWrongMode,
    MStringErrorLimit,
    MStringEnterPass,
    1000)
);

class StringNumeric : public TestWithParam<unsigned int>
{
protected:
  void SetUp() override {
    TestWithParam<unsigned int>::SetUp();

    collected_text_reset();
    collected_alt_text_reset();
  }
  void TearDown() override {
    collected_text_reset();
    collected_alt_text_reset();

    TestWithParam<unsigned int>::TearDown();
  }
};

INSTANTIATE_TEST_CASE_P(
  StringNumbers,
  StringNumeric,
  Values(
    0u,
    1u,
    7u,
    9u,
    10u,
    19u,
    67u,
    99u,
    100u,
    109u,
    147u,
    999u,
    1000u,
    9999u,
    10000u,
    99999u,
    100000u,
    999999u,
    1000000u,
    9999999u,
    10000000u,
    99999999u,
    100000000u,
    999999999u,
    1000000000u)
);

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

TEST_F(StringBasic, MStringRSimple)
{
  mprintstr_R("abc");

  ASSERT_EQ(collected_text_length(), 3);
  ASSERT_STREQ(collected_text(), "abc");
}

TEST_F(AltStringBasic, MAltStringRSimple)
{
  mprintstr_R("abc");

  ASSERT_EQ(collected_alt_text_length(), 3);
  ASSERT_STREQ(collected_alt_text(), "abc");
}

TEST_P(StringWithIds, MString)
{
  ASSERT_STREQ(mstring(GetParam()), ExpectedBaseString());
}

TEST_P(StringWithIds, MError)
{
  merror(GetParam());
  const std::string &expected = std::string("Error: ") + ExpectedBaseString() + "\r\n";
  ASSERT_STREQ(collected_text(), expected.data());
}

TEST_P(StringWithIds, MWarning)
{
  mwarning(GetParam());
  const std::string &expected = std::string("Warning: ") + ExpectedBaseString() + "\r\n";
  ASSERT_STREQ(collected_text(), expected.data());
}

TEST_P(StringNumeric, SimpleZero)
{
  mprint_uintd(GetParam(), 0);
  const std::string &expected = std::to_string(GetParam());
  ASSERT_STREQ(collected_text(), expected.data());
}

TEST_P(StringNumeric, SimpleOne)
{
  mprint_uintd(GetParam(), 1);
  const std::string &expected = std::to_string(GetParam());
  ASSERT_STREQ(collected_text(), expected.data());
}

TEST_P(StringNumeric, SimpleTwo)
{
  mprint_uintd(GetParam(), 2);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%02u", GetParam());
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleFive)
{
  mprint_uintd(GetParam(), 5);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%05u", GetParam());
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleTen)
{
  mprint_uintd(GetParam(), 10);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%010u", GetParam());
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleFifteen)
{
  mprint_uintd(GetParam(), 15);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%015u", GetParam());
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleTwenty)
{
  mprint_uintd(GetParam(), 20);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%020u", GetParam());
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint8Skip)
{
  mprint_uint8(GetParam(), true);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%X", static_cast<uint8_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint8NoSkip)
{
  mprint_uint8(GetParam(), false);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%02X", static_cast<uint8_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint16Skip)
{
  mprint_uint16(GetParam(), true);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%X", static_cast<uint16_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint16NoSkip)
{
  mprint_uint16(GetParam(), false);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%04X", static_cast<uint16_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint32Skip)
{
  mprint_uint32(GetParam(), true);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%X", static_cast<uint32_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint32NoSkip)
{
  mprint_uint32(GetParam(), false);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%08X", static_cast<uint32_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint64Skip)
{
  mprint_uint64(GetParam(), true);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%X", static_cast<uint32_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_P(StringNumeric, SimpleUint64NoSkip)
{
  mprint_uint64(GetParam(), false);

  char buffer[32] = {0};
  snprintf(buffer, sizeof (buffer), "%016X", static_cast<uint32_t>(GetParam()));
  ASSERT_STREQ(collected_text(), buffer);
}

TEST_F(StringBasic, MPrintExprBasicEscapes)
{
  const char *const expected = "abc\r\ndef\t\t\tghi\r\r\n\nq\\wezxrty";
  const char *const original = "abc\\r\\ndef\\t\\t\\tghi\\r\\r\\n\\nq\\\\we\\z\\xrty";

  mprintexpr(original);
  ASSERT_STREQ(collected_text(), expected);
}

TEST_F(StringBasic, MPrintExprNull)
{
  mprintexpr(NULL);
  ASSERT_STREQ(collected_text(), "");
}

TEST_F(StringBasic, MPrintDumpBufferBasicNullNoAddress)
{
  mprint_dump_buffer(0, NULL, false);

  ASSERT_STREQ(collected_text(), "");
}

TEST_F(StringBasic, MPrintDumpBufferBasicNullWithAddress)
{
  mprint_dump_buffer(0, NULL, true);

  ASSERT_STREQ(collected_text(), "");
}

TEST_F(StringBasic, MPrintDumpBufferBasicNullNoAddressWithLength)
{
  mprint_dump_buffer(100, NULL, false);

  ASSERT_STREQ(collected_text(), "");
}

TEST_F(StringBasic, MPrintDumpBufferBasicNullWithAddressWithLength)
{
  mprint_dump_buffer(100, NULL, true);

  ASSERT_STREQ(collected_text(), "");
}

TEST_F(StringBasic, MPrintDumpShortBufferNoAddress)
{
  mprint_dump_buffer(1, reinterpret_cast<const uint8_t *>("A"), false);

  ASSERT_STREQ(collected_text(), "41 \r\n");
}

TEST_F(StringBasic, MPrintDumpShortBufferWithAddress)
{
  mprint_dump_buffer(1, reinterpret_cast<const uint8_t *>("A"), true);

  ASSERT_STREQ(collected_text(), "00000000  41 \r\n");
}

TEST_F(StringBasic, MPrintDumpLongBufferNoAddress)
{
  mprint_dump_buffer(20, reinterpret_cast<const uint8_t *>("1234567890ABCDEFGHIJ"), false);

  ASSERT_STREQ(collected_text(), "31 32 33 34 35 36 37 38 39 30 41 42 43 44 45 46 \r\n47 48 49 4A \r\n");
}

TEST_F(StringBasic, MPrintDumpLongBufferWithAddress)
{
  mprint_dump_buffer(20, reinterpret_cast<const uint8_t *>("1234567890ABCDEFGHIJ"), true);

  ASSERT_STREQ(collected_text(), "00000000  31 32 33 34 35 36 37 38 39 30 41 42 43 44 45 46 \r\n00000010  47 48 49 4A \r\n");
}