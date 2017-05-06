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

#include <CUnit/Basic.h>
#include <CUnit/Automated.h>

static void mcode_parser_parser_tests(void);
static void mcode_parser_string_tests(void);

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
  if (!CU_add_test(pSuite, "", mcode_parser_string_tests)) {
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
}

void mcode_parser_string_tests(void)
{
  CU_ASSERT_EQUAL(mparser_strcmp("abc", 3, "abc"), 0);
  CU_ASSERT_NOT_EQUAL(mparser_strcmp("abc", 3, "abd"), 0);
  CU_ASSERT_EQUAL(mparser_strcmp("abc123", 3, "abc"), 0);
  CU_ASSERT_NOT_EQUAL(mparser_strcmp("abc123", 3, "abd"), 0);
}
