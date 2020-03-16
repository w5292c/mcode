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

#include "scheduler.h"

#include <gtest/gtest.h>

using namespace testing;

class SchedulerTestBasic : public Test
{
protected:
  void SetUp() override {
    _handler1_count = 0;
    _handler2_count = 0;
    scheduler_init();
    scheduler_add(handler1);
    scheduler_add(handler2);
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

protected:
  static int _handler1_count;
  static int _handler2_count;
};

int SchedulerTestBasic::_handler1_count = 0;
int SchedulerTestBasic::_handler2_count = 0;

class SchedulerTestAddHandler : public Test
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

int SchedulerTestAddHandler::_handler1_count = 0;
int SchedulerTestAddHandler::_handler2_count = 0;

TEST_F(SchedulerTestBasic, MPrintStrSimple)
{
  scheduler_start();
}

TEST_F(SchedulerTestAddHandler, MPrintStrSimple)
{
  scheduler_start();
}
