/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Alexander Chumakov
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

#include "mcode-config.h"

#include "utils.h"
#include "hw-rtc.h"
#include "mparser.h"
#include "mstring.h"
#include "mcode-config.h"

#include "hw-uart.h"
#include "gsm-engine-uart2.c"

#include "librock_sha256.c"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

static uint32_t TheCounter = 0;
static bool TheFinished = false;

static char TheTestBuffer[2048];
static size_t TheTestBufferLength = 0;

const char TheLongTestString[] =
  "The MIT License (MIT)\n\n"
  "Copyright (c) 2014 Alexander Chumakov\n\n"
  "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
  "of this software and associated documentation files (the \"Software\"), to deal\n"
  "in the Software without restriction, including without limitation the rights\n"
  "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
  "copies of the Software, and to permit persons to whom the Software is\n"
  "furnished to do so, subject to the following conditions:\n\n"
  "The above copyright notice and this permission notice shall be included in all\n"
  "copies or substantial portions of the Software.\n\n"
  "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
  "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
  "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
  "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
  "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
  "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
  "SOFTWARE.\n\n";

#ifdef MCODE_RANDOM_DATA
static uint8_t TheRands[] = MCODE_RANDOM_BYTES;
#endif /* MCODE_RANDOM_DATA */

static void mcode_pdu_tests(void);
static void mcode_date_time_tests(void);
static void mcode_mparse_next_new(void);
static void mcode_mparse_next_errors(void);
static void mcode_parser_parser_tests(void);
static void mcode_parser_string_tests(void);
static void mcode_security_sha256_tests(void);
static void mcode_common_gsm_engine_send_fstring_tests(void);
static void mcode_common_gsm_engine_send_cmd_raw_tests(void);

static void mcode_hw_uart_handler_test(char *data, size_t length);
static const char *mcode_handler_simple(MParserEvent event, const char *str, size_t length, int32_t value);
static const char *mcode_handler_read_sms(MParserEvent event, const char *str, size_t length, int32_t value);

int main(void)
{
  CU_pSuite pSuite = NULL;

#ifdef MCODE_RANDOM_DATA
  printf("Random data (%d bytes): ", MCODE_RANDOM_BYTES_COUNT);
  for (int i = 0; i < MCODE_RANDOM_BYTES_COUNT; ++i) {
    printf("0x%02x, ", TheRands[i]);
  }
  printf("\n");
#endif /* MCODE_RANDOM_DATA */

  /* Initialize the CUnit test registry */
  if (CUE_SUCCESS != CU_initialize_registry()) {
    return CU_get_error();
  }

  /* Add a suite to the registry */
  pSuite = CU_add_suite("Suite_mcode_parser", NULL, NULL);
  if (!pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Add our tests to the suite */
  if (!CU_add_test(pSuite, "MCODE parser test cases", mcode_parser_parser_tests)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "MCODE parser string utility test cases", mcode_parser_string_tests)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "MCODE date/time test cases", mcode_date_time_tests)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "MCODE PDU test cases", mcode_pdu_tests)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "GSM engine test cases", mcode_common_gsm_engine_send_fstring_tests)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "GSM engine send raw test cases", mcode_common_gsm_engine_send_cmd_raw_tests)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "Security:SHA256 test cases", mcode_security_sha256_tests)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "mparse_next API tests", mcode_mparse_next_new)) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if (!CU_add_test(pSuite, "mparse_next error cases", mcode_mparse_next_errors)) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  /* Run all tests using the basic interface */
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  printf("\n");
  CU_basic_show_failures(CU_get_failure_list());
  printf("\n\n");

  /* Run all tests using the automated interface */
  CU_automated_run_tests();
  CU_list_tests_to_file();

  /* Clean up registry and return */
  unsigned int failedTests = CU_get_number_of_tests_failed();
  CU_cleanup_registry();

  return CU_get_error();
}

void mcode_parser_parser_tests(void)
{
  const char *TestString = "AT\r\n";
  TheCounter = 0;
  mparser_parse(TestString, strlen(TestString), &mcode_handler_simple);
  CU_ASSERT_EQUAL(TheCounter, 5);

  TestString = "+CMGR: 1,\"+70001112233\",,10\r";
  TheCounter = 0;
  mparser_parse(TestString, strlen(TestString), &mcode_handler_simple);
  CU_ASSERT_EQUAL(TheCounter, 13);

  TheFinished = false;
  TestString = "+CMGR: 12,\"+70001234567\",,12\rline 1\rline 2\rline 3\r\rOK\r";
  TheCounter = 0;
  mparser_parse(TestString, strlen(TestString), &mcode_handler_read_sms);
  CU_ASSERT_EQUAL(TheCounter, 19);
}

void mcode_parser_string_tests(void)
{
  CU_ASSERT_EQUAL(mparser_strcmp("abc", 3, "abc"), 0);
  CU_ASSERT_NOT_EQUAL(mparser_strcmp("abc", 3, "abd"), 0);
  CU_ASSERT_EQUAL(mparser_strcmp("abc123", 3, "abc"), 0);
  CU_ASSERT_NOT_EQUAL(mparser_strcmp("abc123", 3, "abd"), 0);
}

const char *mcode_handler_simple(MParserEvent event, const char *str, size_t length, int32_t value)
{
  ++TheCounter;

  switch (event) {
  case EParserEventNull:
    fprintf(stdout, ">>> null\n");
    break;
  case EParserEventBegin:
    fprintf(stdout, ">>> begin\n");
    break;
  case EParserEventToken: {
    char *str1 = strndup(str, length);
    fprintf(stdout, ">>> token: \"%s\"\n", str1);
    free(str1);
    }
    break;
  case EParserEventString: {
    char *str1 = strndup(str, length);
    fprintf(stdout, ">>> string(%zd): \"%s\"\n", length, str1);
    free(str1);
    }
    break;
  case EParserEventNumber: {
    char *str1 = strndup(str, length);
    fprintf(stdout, ">>> number: \"%s\" - [%d]\n", str1, value);
    free(str1);
    }
    break;
  case EParserEventPunct:
    fprintf(stdout, ">>> punct: %d-[%c]\n", value, value);
    break;
  case EParserEventSepWhitespace:
    fprintf(stdout, ">>> whitespace: %zd\n", length);
    break;
  case EParserEventSepEndOfLine:
    fprintf(stdout, ">>> new-line: %zd-[%d]\n", length, value);
    break;
  case EParserEventEnd:
    fprintf(stdout, ">>> end\n");
    break;
  default:
    fprintf(stdout, ">>> Unknown event\n");
    break;
  }

  return NULL;
}

const char *mcode_handler_read_sms(MParserEvent event, const char *str, size_t length, int32_t value)
{
  ++TheCounter;

  switch (event) {
  case EParserEventNull:
    fprintf(stdout, ">>> null\n");
    break;
  case EParserEventBegin:
    fprintf(stdout, ">>> begin\n");
    break;
  case EParserEventToken: {
    char *str1 = strndup(str, length);
    fprintf(stdout, ">>> token: \"%s\"\n", str1);
    free(str1);
    }
    break;
  case EParserEventString: {
    char *str1 = strndup(str, length);
    fprintf(stdout, ">>> string(%zd): \"%s\"\n", length, str1);
    free(str1);
    }
    break;
  case EParserEventNumber: {
    char *str1 = strndup(str, length);
    fprintf(stdout, ">>> number: \"%s\" - [%d]\n", str1, value);
    free(str1);
    }
    break;
  case EParserEventPunct:
    fprintf(stdout, ">>> punct: %d-[%c]\n", value, value);
    break;
  case EParserEventSepWhitespace:
    fprintf(stdout, ">>> whitespace: %zd\n", length);
    break;
  case EParserEventSepEndOfLine: {
    if (!TheFinished) {
    const char *ch = memchr(str, '\r', length);
    if (ch) {
      const size_t lineSz = (ch - str);
      CU_ASSERT(lineSz < length);
      char *line = strndup(str, lineSz);
      fprintf(stdout, ">>> new-line: chars-left: %zd, value: %d, line: \"%s\"\n", length, value, line);
      free(line);
      if (!lineSz) {
        ch = NULL;
        TheFinished = true;
      }
    } else {
      fprintf(stdout, ">>> new-line: chars-left: %zd, value: %d\n", length, value);
    }
    return ch;
    } else {
      fprintf(stdout, ">>> new-line: chars-left: %zd, value: %d\n", length, value);
      return NULL;
    }
    }
    break;
  case EParserEventEnd:
    fprintf(stdout, ">>> end\n");
    break;
  default:
    fprintf(stdout, ">>> Unknown event\n");
    break;
  }

  return NULL;
}

void mcode_date_time_tests(void)
{
  MTime time;
  MDate date;
  uint32_t mtime;

  /* Check initial time */
  memset(&time, 0, sizeof (time));
  rtc_get_time(0, &time);
  CU_ASSERT_EQUAL(time.hours, 0);
  CU_ASSERT_EQUAL(time.minutes, 0);
  CU_ASSERT_EQUAL(time.seconds, 0);

  /* Check initial date */
  memset(&date, 0, sizeof (date));
  rtc_get_date(0, &date);
  CU_ASSERT_EQUAL(date.day, 1);
  CU_ASSERT_EQUAL(date.year, 2001);
  CU_ASSERT_EQUAL(date.month, 1);
  CU_ASSERT_EQUAL(date.dayOfWeek, 1);

  /* Check time '19-Sep-2017, 23:41:59' */
  memset(&time, 0, sizeof (time));
  rtc_get_time(527557319, &time);
  CU_ASSERT_EQUAL(time.hours, 23);
  CU_ASSERT_EQUAL(time.minutes, 41);
  CU_ASSERT_EQUAL(time.seconds, 59);

  /* Check date '19-Sep-2017, 23:41:59' */
  memset(&date, 0, sizeof (date));
  rtc_get_date(527557319, &date);
  CU_ASSERT_EQUAL(date.day, 19);
  CU_ASSERT_EQUAL(date.year, 2017);
  CU_ASSERT_EQUAL(date.month, 9);
  CU_ASSERT_EQUAL(date.dayOfWeek, 2);

  /* Check time '31-Dec-2016, 23:59:59' */
  memset(&time, 0, sizeof (time));
  rtc_get_time(504921599, &time);
  CU_ASSERT_EQUAL(time.hours, 23);
  CU_ASSERT_EQUAL(time.minutes, 59);
  CU_ASSERT_EQUAL(time.seconds, 59);

  /* Check date '31-Dec-2016, 23:59:59' */
  memset(&date, 0, sizeof (date));
  rtc_get_date(504921599, &date);
  CU_ASSERT_EQUAL(date.day, 31);
  CU_ASSERT_EQUAL(date.year, 2016);
  CU_ASSERT_EQUAL(date.month, 12);
  CU_ASSERT_EQUAL(date.dayOfWeek, 6);

  /* Check convertion to 'mtime' for '31-Dec-2016, 23:59:59' */
  mtime = rtc_to_mtime(&date, &time);
  CU_ASSERT_EQUAL(mtime, 504921599);

  /* Check time '1-Jan-2017, 0:00:00' */
  memset(&time, 0, sizeof (time));
  rtc_get_time(504921600, &time);
  CU_ASSERT_EQUAL(time.hours, 0);
  CU_ASSERT_EQUAL(time.minutes, 0);
  CU_ASSERT_EQUAL(time.seconds, 0);

  /* Check date '1-Jan-2017, 0:00:00' */
  memset(&date, 0, sizeof (date));
  rtc_get_date(504921600, &date);
  CU_ASSERT_EQUAL(date.day, 1);
  CU_ASSERT_EQUAL(date.year, 2017);
  CU_ASSERT_EQUAL(date.month, 1);
  CU_ASSERT_EQUAL(date.dayOfWeek, 7);

  /* Check convertion to 'mtime' for '1-Jan-2017, 0:00:00' */
  mtime = rtc_to_mtime(&date, &time);
  CU_ASSERT_EQUAL(mtime, 504921600);
}

void mcode_pdu_tests(void)
{
  char buffer[256] = { 0 };

  size_t length = 0;
  bool res = from_pdu_7bit("F4F29C0E", -1, buffer, sizeof (buffer), &length);
  CU_ASSERT(res);
  CU_ASSERT_STRING_EQUAL(buffer, "test");

  length = 0;
  memset(buffer, 0, sizeof (buffer));
  res = from_pdu_7bit("F3B29BDC4ABBCD6F50AC3693B14022F2DB5D16B140381A", -1, buffer, sizeof (buffer), &length);
  CU_ASSERT(res);
  CU_ASSERT_STRING_EQUAL(buffer, "send-info 1532, \"done\", 84");
}

void mcode_common_gsm_engine_send_fstring_tests(void)
{
  memset(TheTestBuffer, 0, sizeof (TheTestBuffer));
  TheTestBufferLength = 0;

  /* Simple 4-character string */
  CU_ASSERT_EQUAL(TheTestBufferLength, 0);
  gsm_send_fstring("abcd");
  CU_ASSERT_EQUAL(TheTestBufferLength, 4);
  CU_ASSERT_STRING_EQUAL(TheTestBuffer, "abcd");

  memset(TheTestBuffer, 0, sizeof (TheTestBuffer));
  TheTestBufferLength = 0;
  /* Escape sequences */
  gsm_send_fstring("\\a\\b\\r\\n\\e\\v\\t\\f\\\\\\0");
  CU_ASSERT_EQUAL(TheTestBufferLength, 10);
  CU_ASSERT_STRING_EQUAL(TheTestBuffer, "\a\b\r\n\e\v\t\f\\\0");
  CU_ASSERT_EQUAL(TheTestBuffer[TheTestBufferLength - 2], '\\');
  CU_ASSERT_EQUAL(TheTestBuffer[TheTestBufferLength - 1], '\0');

  memset(TheTestBuffer, 0, sizeof (TheTestBuffer));
  TheTestBufferLength = 0;
  /* Zero-character in the middle of a string */
  gsm_send_fstring("prefix\\0postfix");
  CU_ASSERT_EQUAL(TheTestBufferLength, 14);
  CU_ASSERT_STRING_EQUAL(TheTestBuffer, "prefix");
  CU_ASSERT_EQUAL(TheTestBuffer[5], 'x');
  CU_ASSERT_EQUAL(TheTestBuffer[6], '\0');
  CU_ASSERT_EQUAL(TheTestBuffer[7], 'p');
  CU_ASSERT_STRING_EQUAL(TheTestBuffer + 7, "postfix");
}

void mcode_common_gsm_engine_send_cmd_raw_tests(void)
{
  memset(TheTestBuffer, 0, sizeof (TheTestBuffer));
  TheTestBufferLength = 0;

  /* Simple 4-character string */
  CU_ASSERT_EQUAL(TheTestBufferLength, 0);
  gsm_send_cmd_raw("abcd");
  CU_ASSERT_EQUAL(TheTestBufferLength, 5);
  CU_ASSERT_STRING_EQUAL(TheTestBuffer, "abcd\r");

  memset(TheTestBuffer, 0, sizeof (TheTestBuffer));
  TheTestBufferLength = 0;
  /* Escape sequences */
  gsm_send_cmd_raw("\\a\\b\\r\\n\\e\\v\\t\\f\\\\\\0");
  CU_ASSERT_EQUAL(TheTestBufferLength, 11);
  CU_ASSERT_STRING_EQUAL(TheTestBuffer, "\a\b\r\n\e\v\t\f\\\0");
  CU_ASSERT_EQUAL(TheTestBuffer[TheTestBufferLength - 3], '\\');
  CU_ASSERT_EQUAL(TheTestBuffer[TheTestBufferLength - 2], '\0');
  CU_ASSERT_EQUAL(TheTestBuffer[TheTestBufferLength - 1], '\r');

  memset(TheTestBuffer, 0, sizeof (TheTestBuffer));
  TheTestBufferLength = 0;
  /* Zero-character in the middle of a string */
  gsm_send_cmd_raw("prefix\\0postfix");
  CU_ASSERT_EQUAL(TheTestBufferLength, 15);
  CU_ASSERT_STRING_EQUAL(TheTestBuffer, "prefix");
  CU_ASSERT_EQUAL(TheTestBuffer[5], 'x');
  CU_ASSERT_EQUAL(TheTestBuffer[6], '\0');
  CU_ASSERT_EQUAL(TheTestBuffer[7], 'p');
  CU_ASSERT_STRING_EQUAL(TheTestBuffer + 7, "postfix\r");
}

void mcode_security_sha256_tests(void)
{
  const uint8_t empty_str_expected_sha256[] = {
    0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
    0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55,
  };
  uint8_t md[MD_LENGTH_SHA256];
  memset(md, 0, MD_LENGTH_SHA256);
  sha256("", 0, md);
  CU_ASSERT_EQUAL(memcmp(md, empty_str_expected_sha256, MD_LENGTH_SHA256), 0);

  const uint8_t abcd1234_str_expected_sha256[] = {
    0xe9, 0xce, 0xe7, 0x1a, 0xb9, 0x32, 0xfd, 0xe8, 0x63, 0x33, 0x8d, 0x08, 0xbe, 0x4d, 0xe9, 0xdf,
    0xe3, 0x9e, 0xa0, 0x49, 0xbd, 0xaf, 0xb3, 0x42, 0xce, 0x65, 0x9e, 0xc5, 0x45, 0x0b, 0x69, 0xae,
  };
  memset(md, 0, MD_LENGTH_SHA256);
  sha256("abcd1234", 8, md);
  CU_ASSERT_EQUAL(memcmp(md, abcd1234_str_expected_sha256, MD_LENGTH_SHA256), 0);

  const uint8_t long_test_str_expected_sha256[] = {
    0x09, 0xbf, 0x00, 0xef, 0xa6, 0x49, 0x30, 0x2f, 0x93, 0x54, 0xb3, 0x86, 0x13, 0x13, 0xe2, 0xf1,
    0xfc, 0x5b, 0xd0, 0x71, 0x7c, 0xa7, 0x7e, 0xf2, 0xd8, 0xfb, 0xd4, 0xf5, 0x5a, 0x17, 0x48, 0x9c,
  };
  memset(md, 0, MD_LENGTH_SHA256);
  sha256(TheLongTestString, sizeof (TheLongTestString) - 1, md);
  CU_ASSERT_EQUAL(memcmp(md, long_test_str_expected_sha256, MD_LENGTH_SHA256), 0);
}

void uart_write_char(char ch)
{
}

void uart2_write_char(char ch)
{
  TheTestBuffer[TheTestBufferLength++] = ch;
}

void hw_gsm_init(void)
{
}

void hw_uart2_set_callback(hw_uart_handler cb)
{
}

void mcode_hw_uart_handler_test(char *data, size_t length)
{
}

void hw_gsm_power(bool on)
{
}

void line_editor_uart_start(void)
{
}

void mcode_mparse_next_new(void)
{
  const char *str;
  const char *token;
  size_t length;
  TokenType type;
  uint32_t value;
  const char test_string[] = "\"string12\", token56, 12345";

  value = 0;
  token = NULL;
  str = test_string;
  length = strlen(test_string);
  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenString);
  CU_ASSERT_EQUAL(str, test_string + 10);
  CU_ASSERT_EQUAL(length, 16);
  CU_ASSERT_EQUAL(mparser_strcmp(token, value, "string12"), 0);

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenPunct);
  CU_ASSERT_EQUAL(str, test_string + 11);
  CU_ASSERT_EQUAL(length, 15);
  CU_ASSERT_EQUAL(value, ',');

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenWhitespace);
  CU_ASSERT_EQUAL(str, test_string + 12);
  CU_ASSERT_EQUAL(length, 14);
  CU_ASSERT_EQUAL(value, ' ');

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenId);
  CU_ASSERT_EQUAL(str, test_string + 19);
  CU_ASSERT_EQUAL(length, 7);
  CU_ASSERT_EQUAL(value, 7);
  CU_ASSERT_EQUAL(mparser_strcmp(token, value, "token56"), 0);

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenPunct);
  CU_ASSERT_EQUAL(str, test_string + 20);
  CU_ASSERT_EQUAL(length, 6);
  CU_ASSERT_EQUAL(value, ',');

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenWhitespace);
  CU_ASSERT_EQUAL(str, test_string + 21);
  CU_ASSERT_EQUAL(length, 5);
  CU_ASSERT_EQUAL(value, ' ');

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenInt);
  CU_ASSERT_EQUAL(str, test_string + 26);
  CU_ASSERT_EQUAL(length, 0);
  CU_ASSERT_EQUAL(value, 12345);

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenEnd);
  CU_ASSERT_EQUAL(str, test_string + 26);
  CU_ASSERT_EQUAL(length, 0);

  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenEnd);
  CU_ASSERT_EQUAL(str, test_string + 26);
  CU_ASSERT_EQUAL(length, 0);
}

void mcode_mparse_next_errors(void)
{
  TokenType type;
  const char *str;
  size_t length;
  uint32_t value;
  const char *token;
  const char tstr[] = "123a, \"string\"";

  str = tstr;
  length = strlen(str);
  type = next_token(&str, &length, &token, &value);
  CU_ASSERT_EQUAL(type, TokenError);
  CU_ASSERT_EQUAL(str, tstr + 4);
  CU_ASSERT_EQUAL(token, tstr);
  CU_ASSERT_EQUAL(length, 10);
  CU_ASSERT_EQUAL(value, 4);
}
