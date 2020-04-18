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

#include "mtick.h"
#include "mtimer.h"
#include "scheduler.h"

#include <gtest/gtest.h>

using namespace testing;

class TimerTestBasic : public Test
{
protected:
  void SetUp() override {
    _handler1_count = 0;
    _handler2_count = 0;
    scheduler_init();
    mtick_init();
    mtimer_init();

    mtimer_add(handler1, 10);
    mtimer_add_periodic(handler2, 20, 10);
  }
  void TearDown() override {
    mtimer_deinit();
    mtick_deinit();
    scheduler_deinit();
  }

  static bool handler1(void) {
    ++_handler1_count;

    return false;
  }
  static bool handler2(void) {
    ++_handler2_count;
    if (_handler2_count == 3) {
      return false;
    }

    return true;
  }

protected:
  static int _handler1_count;
  static int _handler2_count;
};

int TimerTestBasic::_handler1_count = 0;
int TimerTestBasic::_handler2_count = 0;

class TimerTestAddHandler : public Test
{
protected:
  void SetUp() override {
    _handler1_count = 0;
    _handler2_count = 0;
    scheduler_init();
    scheduler_add(handler1);
    scheduler_add(handler2);
    scheduler_add(handler3);
    scheduler_add(handler3);
    scheduler_add(handler3);
    scheduler_add(handler3);
    scheduler_add(handler3);
    scheduler_add(handler3);
    scheduler_add(handler3);
  }
  void TearDown() override {
    scheduler_deinit();
  }

  static void handler1(void) {
    ++_handler1_count;
    if (_handler1_count == 5) {
      scheduler_start();
    }
  }
  static void handler2(void) {
    ++_handler2_count;
    if (_handler2_count > 10 && _handler2_count > 10) {
      scheduler_stop();
    }
  }
  static void handler3(void) {
  }

protected:
  static int _handler1_count;
  static int _handler2_count;
};

int TimerTestAddHandler::_handler1_count = 0;
int TimerTestAddHandler::_handler2_count = 0;

TEST_F(TimerTestBasic, MPrintStrSimple)
{
  usleep(100000);
}

TEST_F(TimerTestAddHandler, MPrintStrSimple)
{
  scheduler_start();
}
