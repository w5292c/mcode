/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 Alexander Chumakov
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

#include "mstring.h"
#include "mcode-config.h"

#ifndef MCODE_TICKS_COUNT
#define MCODE_TICKS_COUNT (8)
#endif /* MCODE_TICKS_COUNT */

static uint8_t TheIterator;
static uint8_t TheRunFlags;
static uint8_t ClientsNumber;
static uint8_t TheNextRunFlag;
static mcode_tick TheApplicationTicks[MCODE_TICKS_COUNT];

void scheduler_init(void)
{
  TheRunFlags = 0;
  TheNextRunFlag = 1;
  ClientsNumber = 0;
}

void scheduler_deinit(void)
{
}

void scheduler_start(uint8_t *id)
{
  /* Minimizing the call stack usage here */
  if (!id) {
    merror(MStringWrongArgument);
    return;
  }
  if (!TheNextRunFlag) {
    /* Return invalid/neutral ID */
    *id = 0;
    /* Too many starts, TheNextRunFlag' overflowed */
    merror(MStringInternalError);
    return;
  }

  *id = TheNextRunFlag;
  TheNextRunFlag = (TheNextRunFlag << 1);
  TheRunFlags |= *id;

  while (TheRunFlags & *id) {
    for (TheIterator = 0; TheIterator < ClientsNumber; ++TheIterator) {
      if (TheApplicationTicks[TheIterator]) {
        (*TheApplicationTicks[TheIterator])();
      }
    }
  }
}

void scheduler_stop(uint8_t id)
{
  TheRunFlags = (TheRunFlags & (~id));
}

void scheduler_add(mcode_tick tick)
{
  if (ClientsNumber < MCODE_TICKS_COUNT) {
    TheApplicationTicks[ClientsNumber++] = tick;
  } else {
    /*! @todo add assert(false) here */
    merror(MStringInternalError);
    mprintstrln(PSTR("Error: no room for scheduler handler"));
  }
}
