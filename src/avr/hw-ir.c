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

#include "hw-ir.h"

#include "mglobal.h"
#include "mstring.h"

#include <string.h>
#include <stdbool.h>
#include <avr/interrupt.h>

static volatile uint8_t buffer[128];
static volatile uint32_t count;
static volatile bool CurrentLevel;

void ir_init(void)
{
  /* Configure INT2 pin (PB2) as input */
  DDRB &= ~_BV(DDB2);
  /* No pull-ups for PB2 */
  PORTB &= ~_BV(PB2);

  /* HIGH on no-activity */
  CurrentLevel = true;
  /* INT2 on falling edge */
  MCUCSR &= ~_BV(ISC2);
  /* Reset flag */
  GIFR |= _BV(INTF2);

  TCCR1B |= _BV(CS10);

  /* Enable external interrupt INT2 */
  GICR |= _BV(INT2);
}

uint32_t ir_get(void)
{
  return (PINB & PINB2) ? count + 1000 : count;
}

void ir_dump(void)
{
  mprintstrln(PSTR("Buffer:"));
  mprint_dump_buffer(127, (const uint8_t *)buffer, true);
  memset((void *)buffer, 0, 128);
  count = 0;
}

ISR(INT2_vect)
{
  if (CurrentLevel) {
    CurrentLevel = false;
    /* INT2 on rising edge */
    MCUCSR |= _BV(ISC2);
  } else {
    CurrentLevel = true;
    /* INT2 on falling edge */
    MCUCSR &= ~_BV(ISC2);
  }
  /* Reset flag */
  GIFR |= _BV(INTF2);
  buffer[count] = TCNT1L;
  TCNT1L = 0;
  ++count;
  if (count >= 127) {
    count = 0;
  }
}
