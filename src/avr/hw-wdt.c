/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Alexander Chumakov
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

#include "hw-wdt.h"

#include "mtick.h"

#include <avr/io.h>
#include <avr/wdt.h>

static uint8_t TheResetReason;

static void wdt_tick(void);

void wdt_init(void)
{
  TheResetReason = MCUSR;
  MCUSR = 0;
  wdt_enable(WDTO_2S);
  wdt_reset();
  mtick_add(wdt_tick);
}

void wdt_start(void)
{
  wdt_enable(WDTO_2S);
  wdt_reset();
}

void wdt_stop(void)
{
  wdt_reset();
  wdt_disable();
}

uint8_t wdt_reset_reason(void)
{
  return TheResetReason;
}

void wdt_tick(void)
{
  wdt_reset();
}
