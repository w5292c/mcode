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

#include "mparser.h"

#include <stdio.h>
#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

static uint32_t TheCounter = 0;

static void mcode_parser_parser_tests(void);
static void mcode_parser_string_tests(void);

static const char *mcode_handler_simple(MParserEvent event, const char *str, size_t length, int32_t value);

int main(void)
{
  CU_pSuite pSuite = NULL;

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
    fprintf(stdout, ">>> string(%d): \"%s\"\n", length, str1);
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
    fprintf(stdout, ">>> whitespace: %d\n", length);
    break;
  case EParserEventSepEndOfLine:
    fprintf(stdout, ">>> new-line: %d-[%d]\n", length, value);
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
