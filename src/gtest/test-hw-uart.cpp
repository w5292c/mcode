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

#include "hw-uart.h"

#include <string.h>
#include <gtest/gtest.h>

using namespace testing;

class HwUart2 : public Test
{
protected:
  void SetUp() override {
    _invoked = false;
    _buffer_length = 0;
    memset(_buffer, 0, sizeof (_buffer));
    hw_uart_init();
    hw_uart2_set_callback(uart_handler);
  }
  void TearDown() override {
    hw_uart_deinit();
  }

  static void uart_handler(const char *data, size_t length) {
    _invoked = true;
    memcpy(_buffer, data, length);
    _buffer_length = length;
  }

  static bool _invoked;
  static char _buffer[512];
  static size_t _buffer_length;
};
bool HwUart2::_invoked = false;
char  HwUart2::_buffer[512] = {0};
size_t HwUart2::_buffer_length = 0;

TEST_F(HwUart2, uart2_receiver)
{
  // 1st case
  uart2_report_new_sample();
  uart2_handle_new_sample('a');
  uart2_report_new_sample();
  uart2_report_new_sample();
  uart2_handle_new_sample('t');
  uart2_report_new_sample();
  uart2_handle_new_sample('d');
  uart2_report_new_sample();
  ASSERT_FALSE(_invoked);
  uart2_handle_new_sample('\r');
  ASSERT_FALSE(_invoked);
  uart2_report_new_sample();
  ASSERT_FALSE(_invoked);
  uart2_handle_new_sample('\n');
  ASSERT_FALSE(_invoked);
  ASSERT_EQ(_buffer_length, 0);
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 3);
  ASSERT_STREQ(_buffer, "atd");
  memset(_buffer, 0, sizeof (_buffer));
  _buffer_length = 0;

  // 2nd case
  _invoked = false;
  uart2_report_new_sample();
  ASSERT_FALSE(_invoked);
  uart2_handle_new_sample('\r');
  uart2_report_new_sample();
  ASSERT_FALSE(_invoked);
  uart2_handle_new_sample('\n');
  ASSERT_FALSE(_invoked);
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 0);

  // 3rd case
  _invoked = false;
  uart2_handle_new_sample('O');
  uart2_report_new_sample();
  uart2_handle_new_sample('K');
  uart2_report_new_sample();
  uart2_handle_new_sample('\r');
  uart2_report_new_sample();
  uart2_handle_new_sample('a');
  uart2_report_new_sample();
  ASSERT_FALSE(_invoked);
  uart2_handle_new_sample('A');
  uart2_report_new_sample();
  uart2_handle_new_sample('\r');
  uart2_report_new_sample();
  uart2_handle_new_sample('\n');
  ASSERT_FALSE(_invoked);
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 1);
  ASSERT_STREQ(_buffer, "A");
  memset(_buffer, 0, sizeof (_buffer));
  _buffer_length = 0;
  _invoked = false;

  // 4th case
  uart2_handle_new_sample('A');
  uart2_handle_new_sample('1');
  uart2_handle_new_sample('\r');
  uart2_handle_new_sample('\n');
  uart2_handle_new_sample('B');
  uart2_handle_new_sample('2');
  uart2_handle_new_sample('\r');
  uart2_handle_new_sample('\n');
  uart2_handle_new_sample('C');
  uart2_handle_new_sample('3');
  uart2_handle_new_sample('\r');
  uart2_handle_new_sample('\n');
  uart2_handle_new_sample('D');
  uart2_handle_new_sample('4');
  uart2_handle_new_sample('\r');
  uart2_handle_new_sample('\n');
  uart2_handle_new_sample('E');
  uart2_handle_new_sample('5');
  uart2_handle_new_sample('\r');
  uart2_handle_new_sample('\n');
  ASSERT_FALSE(_invoked);
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 2);
  /// @todo check which item to remort here
  ASSERT_STREQ(_buffer, "A1");
  memset(_buffer, 0, sizeof (_buffer));
  _buffer_length = 0;
  _invoked = false;
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 2);
  ASSERT_STREQ(_buffer, "B2");
  memset(_buffer, 0, sizeof (_buffer));
  _buffer_length = 0;
  _invoked = false;
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 2);
  ASSERT_STREQ(_buffer, "C3");
  memset(_buffer, 0, sizeof (_buffer));
  _buffer_length = 0;
  _invoked = false;
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 2);
  ASSERT_STREQ(_buffer, "D4");
  memset(_buffer, 0, sizeof (_buffer));
  _buffer_length = 0;
  _invoked = false;
  uart2_report_new_sample();
  ASSERT_FALSE(_invoked);
  uart2_handle_new_sample('G');
  uart2_handle_new_sample('6');
  uart2_handle_new_sample('\r');
  uart2_handle_new_sample('\n');
  ASSERT_FALSE(_invoked);
  uart2_report_new_sample();
  ASSERT_TRUE(_invoked);
  ASSERT_EQ(_buffer_length, 2);
  ASSERT_STREQ(_buffer, "G6");
}
