/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014,2015 Alexander Chumakov
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

#include "hw-leds.h"

#include "mglobal.h"
#include "mstring.h"

static uint8_t TheLedStates = 0;

/*
 * Test code that manages 2 test LEDs connected to PB2, PB3
 */
void leds_init(void)
{
  TheLedStates = 0;
}

void leds_deinit(void)
{
}

void leds_set(int index, int on)
{
  mprintstr(PSTR("Setting LED"));
  mprint_uintd(index, 0);
  mprintstr(PSTR(": "));
  mprintstr(on ? PSTR("ON") : PSTR("OFF"));
  mprint(MStringNewLine);

  if (index >= 0 && index < 8) {
    if (on) {
      TheLedStates |= (1U << index);
    } else {
      TheLedStates &= ~(1U << index);
    }
  }
}

int leds_get(int index)
{
  return (index >=0 && index < 8) ? (TheLedStates & (1U << index)) : 0;
}
