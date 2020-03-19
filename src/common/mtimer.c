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

#include "mtimer.h"

#include "mtick.h"
#include "scheduler.h"

#include <string.h>

#define MCODE_INCORRECT_PERIOD (UINT32_C(0xFFFFFFFF))

/**
 * The timer nodes store the request information
 */
typedef struct {
  /**
    * The time for the next timer handler refering to mcode_count/uptime in milliseconds,
    * \c 0 means - uninitialized/not used
    */
  uint64_t next;
  uint32_t period;
  mcode_exec handler;
} TimerNode;

/**
 * This array stors the timer handlers in sorted order, the handlers to be invoked first go first
 */
static TimerNode TheTimerNodes[MCODE_TIMER_HANDLERS] = {0};

static void mtimer_scheduler_tick(void);
static void mtimer_postprocess_top_node(bool more_work);

static void mtimer_add_handler(mcode_exec task, uint64_t next, uint32_t period);

void mtimer_init(void)
{
  scheduler_add(mtimer_scheduler_tick);
}

void mtimer_deinit(void)
{
  memset(TheTimerNodes, 0, sizeof (TheTimerNodes));
}

void mtimer_add(mcode_exec task, uint32_t start)
{
  mtimer_add_handler(task, mtick_count() + start, MCODE_INCORRECT_PERIOD);
}

void mtimer_add_periodic(mcode_exec task, uint32_t start, uint32_t period)
{
  mtimer_add_handler(task, mtick_count() + start, period);
}

void mtimer_scheduler_tick(void)
{
  if (!TheTimerNodes->next) {
    /* No items in the array, just return */
    return;
  }

  bool more_work;
  enum { count = sizeof (TheTimerNodes)/sizeof (*TheTimerNodes) };
  const uint64_t time = mtick_count();
  while (TheTimerNodes->next && TheTimerNodes->next <= time) {
    more_work = (*TheTimerNodes->handler)();
    mtimer_postprocess_top_node(more_work);
  }
}

void mtimer_postprocess_top_node(bool more_work)
{
  /* Store the top node in a temporary memory */
  TimerNode node;

  node = *TheTimerNodes;
  memmove(TheTimerNodes, TheTimerNodes + 1, sizeof (TimerNode) * (MCODE_TIMER_HANDLERS - 1));

  if (MCODE_INCORRECT_PERIOD != node.period && more_work) {
    mtimer_add_handler(node.handler, node.next + node.period, node.period);
  }
}

void mtimer_add_handler(mcode_exec task, uint64_t next, uint32_t period)
{
  TimerNode *ptr = TheTimerNodes;
  const TimerNode *const end = TheTimerNodes + MCODE_TIMER_HANDLERS;
  while (ptr->next && ptr->next < next) {
    if (ptr == end) {
      /* No more room for handlers */
      return;
    }

    ++ptr;
  }

  const size_t n = ptr - TheTimerNodes; /*< Item index */
  memmove(ptr + 1, ptr, (MCODE_TIMER_HANDLERS - n - 1)*sizeof (TimerNode));
  ptr->next = next;
  ptr->period = period;
  ptr->handler = task;
}
