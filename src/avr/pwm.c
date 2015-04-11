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

#include "pwm.h"

#include <avr/io.h>

void pwm_init(void)
{
  /* Configure Timer1 in PWM Fast mode */
  TCCR1B =
    (0<<WGM13)|(1<<WGM12)|         /*< Mode: Fast PWM, 8 bit */
    (0<<CS12)|(1<<CS11)|(1<<CS10)| /* Prescaler: 64 */
    (0<< ICNC1)|(0<< ICES1);       /*< No extra flags */
  TCCR1A =
    (0<<WGM11)|(1<<WGM10)|         /*< Mode: Fast PWM, 8 bit */
    (1<<COM1A1)|(0<<COM1A0)|       /*< Compare output mode OC1A: non-inverting */
    (1<<COM1B1)|(0<<COM1B0)|       /*< Compare output mode OC1B: non-inverting */
    (0<<FOC1A)|(0<<FOC1B);         /*< No forced output */
  OCR1A = UINT16_C(0);
  OCR1B = UINT16_C(0);
  TCNT1 = UINT16_C(0);
  DDRD |= ((1U << DDD4)|(1U << DDD5)|(1U << DDD7));

  /* Configure Timer2 in PWM fast mode */
  /* Reset the Timer2 counter */
  TCNT2 = UINT8_C(0);
  /* Timer2 compare register: 1001.73913Hz */
  OCR2 = UINT8_C(0);
  /* Set the control register */
  TCCR2 =
    (1<<WGM21)|(1<<WGM20)| /*< Mode: Fast PWM */
    (1<<COM21)|(0<<COM21)| /*< Compare output mode OC2: non-inverting */
    (1<<CS22)|(0<<CS21)|(0<<CS20); /* Prescaler: 64 */
}

void pwm_set(uint8_t id, uint8_t value)
{
  switch (id) {
  case PWM_ID_OC1A:
    OCR1AL = value;
    break;
  case PWM_ID_OC1B:
    OCR1BL = value;
    break;
  case PWM_ID_OC2:
    OCR2 = value;
    break;
  default:
    break;
  }
}
