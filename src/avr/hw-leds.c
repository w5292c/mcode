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

#include "hw-uart.h"

#include <avr/io.h>

inline static uint8_t
leds_get_led_bit(int index)
{
  index = 3 - index;
  switch (index) {
  case 0:
    return _BV(PA4);
  case 1:
    return _BV(PA5);
  case 2:
    return _BV(PA6);
  default:
  case 3:
    return _BV(PA7);
  }
}

/*
 * Test code that manages 2 test LEDs connected to PB2, PB3
 */
void leds_init(void)
{
#if 0
  /* configure PB2, PB3 as outputs */
  DDRD |= ((1U << DDD4)|(1U << DDD5));
  /* turn both LEDs OFF */
  PORTD &= ~((1U << PD4)|(1U << PD5));
#else
  DDRA |= _BV(DDA4)|_BV(DDA5)|_BV(DDA6)|_BV(DDA7);
#endif
}

void leds_deinit(void)
{
}

void leds_set(int index, int on)
{
  const uint8_t ledBit = leds_get_led_bit(index);
  if (on) {
    PORTA |= ledBit;
  } else {
    PORTA &= ~ledBit;
  }
}

int leds_get(int index)
{
  const uint8_t ledBit = leds_get_led_bit(index);
  return (PORTA & ledBit) ? 1 : 0;
}
