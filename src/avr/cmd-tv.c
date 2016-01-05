/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015,2016 Alexander Chumakov
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
  /* Configure external request pin */
  /* Configure PA7 as output */
  DDRA |= (1U << DDA7);
  /* Initially, no external request */
  PORTA &= ~(1U << PA7);
  /* Configure INT0 pin (PD2) as input */
  DDRD &= ~(1U << DDD2);
  /* No pull-ups for PD2 */
  PORTD &= ~(1U << PD2);
  /* Interrupt on any logical change on PD2 (INT0) */
  MCUCR |= (0U<<ISC01)|(1U<<ISC00);
  /* Enable external interrupt INT0 */
  GICR |= (1U<<INT0);
}

bool cmd_engine_tv_ext_request(void)
{
  return (PORTD & (1U << PD2)) ? true : false;
}

void cmd_engine_tv_turn(bool on)
{
  const bool extRequest = (PORTA & (1U << PA7)) ? true : false;
  if (extRequest != on) {
    if (on && cmd_engine_tv_ext_request()) {
      PORTA |= (1U << PA7);
    } else {
      PORTA &= ~(1U << PA7);
    }
  }
}

void cmd_engine_tv_emulate_ext_request(bool on)
{
#if 0
  if (on) {
    PORTD |= (1U << PD2);
  } else {
    PORTD &= ~(1U << PD2);
  }
#endif /* 0 */
}

ISR(INT0_vect)
{
  cmd_engine_tv_ext_req_changed_interrupt();
}
