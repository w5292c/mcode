/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Alexander Chumakov
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

#ifndef MCODE_MTICKS_COUNT
#define MCODE_MTICKS_COUNT (10)
#endif /* MCODE_MTICKS_COUNT */

static volatile uint64_t TheMSecCounter = 0;
static mcode_tick TheTickCallbacks[MCODE_MTICKS_COUNT];

void mtick_init(void)
{
  int i;
  for (i = 0; i < MCODE_MTICKS_COUNT; ++i) {
    TheTickCallbacks[i] = 0;
  }
}

void mtick_deinit(void)
{
}

void mtick_add(mcode_tick tick)
{
  int i;
  for (i = 0; i < MCODE_MTICKS_COUNT; ++i) {
    if (!TheTickCallbacks[i]) {
      TheTickCallbacks[i] = tick;
      break;
    }
  }
}

void mtick_stop(void)
{
}

void mtick_start(void)
{
}

uint64_t mtick_count(void)
{
  return TheMSecCounter;
}

void TIM6_IRQHandler(void)
{
  ++TheMSecCounter;
}
