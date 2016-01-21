/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Alexander Chumakov
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

#include "hw-wdt.h"
#include "scheduler.h"
#include "mcode-config.h"

#include <stdbool.h>
#include <avr/interrupt.h>

#ifndef MCODE_MTICKS_COUNT
#define MCODE_MTICKS_COUNT UINT8_C(2)
#endif /* MCODE_MTICKS_COUNT */

static volatile uint64_t TheMSecCounter = 0;
static volatile bool TheSceduledFlag = false;
static mcode_tick TheTickCallbacks[MCODE_MTICKS_COUNT];

static void mcode_mtick_scheduler_tick(void);

void mtick_init(void)
{
  mcode_scheduler_add(mcode_mtick_scheduler_tick);

  /* Reset the Timer2 counter */
  TCNT0 = 0x00U;
  /* Timer0 compare register (Main clock: 7.3728MHz):
     - 114: 1001.73913Hz
     - 115:  993.10345Hz */
  OCR0 = 115;
  /* Clear OCF2 / clear pending interrupts */
  TIFR  = (1<<OCF0);
  /* Enable Timer2 Compare Interrupt */
  TIMSK = (1<<OCIE0);
  /* Set the control register */
  TCCR0 =
    (1<<WGM01)|(0<<WGM00)| /*< Mode: CTC */
    (0<<COM01)|(0<<COM01)| /*< No port output */
    (0<<CS02)|(1<<CS01)|(1<<CS00); /* Prescaler: 64 */

#ifdef MCODE_WDT
  wdt_init();
  wdt_start();
#endif /* MCODE_WDT */
}

void mtick_deinit(void)
{
}

void mtick_add(mcode_tick tick)
{
  uint8_t i;
  for (i = 0; i < MCODE_MTICKS_COUNT; ++i) {
    if (!TheTickCallbacks[i]) {
      TheTickCallbacks[i] = tick;
      break;
    }
  }
}

void mtick_sleep(uint32_t mticks)
{
  const uint64_t target = TheMSecCounter + mticks + 1;
  while (TheMSecCounter < target) {
#ifdef MCODE_WDT
    wdt_notify();
#endif /* MCODE_WDT */
  }
}

uint64_t mtick_count(void)
{
  return TheMSecCounter;
}

void mcode_mtick_scheduler_tick(void)
{
  if (TheSceduledFlag) {
    TheSceduledFlag = false;
    mcode_tick tick;
    uint8_t i;
    for (i = 0; i < MCODE_MTICKS_COUNT; ++i) {
#ifdef MCODE_WDT
      wdt_notify();
#endif /* MCODE_WDT */
      tick = TheTickCallbacks[i];
      if (!tick) {
        break;
      }

      /* Invoke the tick */
      (*tick)();
    }
  }
}

/* This is a 1 KHz timer2 handler */
ISR(TIMER0_COMP_vect)
{
  ++TheMSecCounter;
  /* set the flag */
  TheSceduledFlag = true;
}
