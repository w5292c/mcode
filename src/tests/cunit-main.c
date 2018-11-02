/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2018 Alexander Chumakov
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
#include "hw-rtc.h"
#include "mparser.h"
#include "scheduler.h"
#include "mcode-config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

static uint32_t TheCounter = 0;
static bool TheFinished = false;

static uint8_t TheSchedulerId1 = 0;
static uint8_t TheSchedulerId2 = 0;
static uint8_t TheSchedulerId3 = 0;
static uint8_t TheSchedulerId4 = 0;
static uint8_t TheSchedulerId5 = 0;
static uint8_t TheSchedulerId6 = 0;
static uint8_t TheSchedulerId7 = 0;
static uint8_t TheSchedulerId8 = 0;
static uint8_t TheSchedulerId9 = 0;
static uint32_t TheSchedulerTestsTickCount = 0;

#ifdef MCODE_RANDOM_DATA
static uint8_t TheRands[] = MCODE_RANDOM_BYTES;
#endif /* MCODE_RANDOM_DATA */

static void scheduler_tests(void);
static void mcode_pdu_tests(void);
static void mcode_date_time_tests(void);
static void mcode_parser_parser_tests(void);
static void mcode_parser_string_tests(void);

static void scheduler_tests_tick(void);
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
  if (!CU_add_test(pSuite, "MCODE Scheduler test cases", scheduler_tests)) {
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
  CU_ASSERT_EQUAL(TheCounter, 12);

  TheFinished = false;
  TestString = "+CMGR: 12,\"+70001234567\",,12\rline 1\rline 2\rline 3\r\rOK\r";
  TheCounter = 0;
  mparser_parse(TestString, strlen(TestString), &mcode_handler_read_sms);
  CU_ASSERT_EQUAL(TheCounter, 18);
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

void scheduler_tests(void)
{
  TheSchedulerId1 = 0;
  TheSchedulerId2 = 0;
  TheSchedulerId3 = 0;
  TheSchedulerId4 = 0;
  TheSchedulerId5 = 0;
  TheSchedulerId6 = 0;
  TheSchedulerId7 = 0;
  TheSchedulerId8 = 0;
  TheSchedulerId9 = 0;
  TheSchedulerTestsTickCount = 0;

  scheduler_init();
  scheduler_add(scheduler_tests_tick);

  CU_ASSERT_EQUAL(TheSchedulerId1, 0);
  scheduler_start(&TheSchedulerId1);
  CU_ASSERT_EQUAL(TheSchedulerId1, 1);

  scheduler_deinit();
  CU_ASSERT_EQUAL(TheSchedulerTestsTickCount, 16);
}

void scheduler_tests_tick(void)
{
  ++TheSchedulerTestsTickCount;

  switch (TheSchedulerTestsTickCount) {
  case 1:
    CU_ASSERT_EQUAL(TheSchedulerId2, 0);
    scheduler_start(&TheSchedulerId2);
    CU_ASSERT_EQUAL(TheSchedulerId2, 2);
    break;
  case 2:
    CU_ASSERT_EQUAL(TheSchedulerId3, 0);
    scheduler_start(&TheSchedulerId3);
    CU_ASSERT_EQUAL(TheSchedulerId3, 4);
    break;
  case 3:
    CU_ASSERT_EQUAL(TheSchedulerId4, 0);
    scheduler_start(&TheSchedulerId4);
    CU_ASSERT_EQUAL(TheSchedulerId4, 8);
    break;
  case 4:
    CU_ASSERT_EQUAL(TheSchedulerId5, 0);
    scheduler_start(&TheSchedulerId5);
    CU_ASSERT_EQUAL(TheSchedulerId5, 16);
    break;
  case 5:
    CU_ASSERT_EQUAL(TheSchedulerId6, 0);
    scheduler_start(&TheSchedulerId6);
    CU_ASSERT_EQUAL(TheSchedulerId6, 32);
    break;
  case 6:
    CU_ASSERT_EQUAL(TheSchedulerId7, 0);
    scheduler_start(&TheSchedulerId7);
    CU_ASSERT_EQUAL(TheSchedulerId7, 64);
    break;
  case 7:
    CU_ASSERT_EQUAL(TheSchedulerId8, 0);
    scheduler_start(&TheSchedulerId8);
    CU_ASSERT_EQUAL(TheSchedulerId8, 128);
    break;
  case 8: {
    uint8_t id = 0xffu;
    CU_ASSERT_EQUAL(id, 0xffu);
    scheduler_start(&id);
    CU_ASSERT_EQUAL(id, 0);
    }
    break;
  case 9:
    scheduler_stop(TheSchedulerId8);
    break;
  case 10:
    scheduler_stop(TheSchedulerId1);
    break;
  case 11:
    scheduler_stop(TheSchedulerId2);
    break;
  case 12:
    scheduler_stop(TheSchedulerId3);
    break;
  case 13:
    scheduler_stop(TheSchedulerId4);
    break;
  case 14:
    scheduler_stop(TheSchedulerId5);
    break;
  case 15:
    scheduler_stop(TheSchedulerId6);
    break;
  case 16:
    scheduler_stop(TheSchedulerId7);
    break;
  default:
    CU_ASSERT(false);
    break;
  }
}

void uart_write_char(char ch)
{
  putchar(ch);
}
