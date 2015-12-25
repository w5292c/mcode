/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Alexander Chumakov
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

#include "cmd-engine.h"

#include <avr/interrupt.h>

void cmd_engine_tv_init_avr(void)
{
  /* Configure INT0 pin (PD2) as output */
  DDRD |= (1U << DDD2);
  /* Set PD3 */
  PORTD |= (1U << PD2);
  MCUCR |= (0U<<ISC00)|(1U<<ISC01);
  GICR |= (1U<<INT0);
}

void cmd_engine_tv_emulate_ext_request(bool on)
{
  if (on) {
    PORTD |= (1U << PD2);
  } else {
    PORTD &= ~(1U << PD2);
  }
}

ISR(INT0_vect)
{
  cmd_engine_tv_ext_req_changed_interrupt();
}
