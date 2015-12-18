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

#include "hw-rtc.h"

#include "hw-twi.h"
#include "hw-uart.h"
#include "scheduler.h"
#include "cmd-engine.h"

#include <avr/interrupt.h>

static bool volatile TheIsrRequest;

static void rtc_alarm_tick(void);
static void rtc_alarm_twi_write_done(bool success);

void rtc_alarm_init(void)
{
  /* Configure INT1 pin (PD3) as input */
  DDRD &= ~(1U << DDD3);
  /* Enable pull-up resistor for PD3 */
  PORTD |= (1U << PD3);
  MCUCR = 0;
  GICR |= (1U<<INT1);
  mcode_scheduler_add(rtc_alarm_tick);
}

void rtc_alarm_deinit(void)
{
}

static uint8_t TheBuffer[2];
void rtc_alarm_tick(void)
{
  if (TheIsrRequest) {
    hw_uart_write_string_P(PSTR("Alarm1: triggered.\r\n"));
    cmd_engine_start();
    /* Clear ALARM1 flag */
    TheIsrRequest = false;
    TheBuffer[0] = 0x0f;
    TheBuffer[1] = 0x88;
    twi_set_write_callback(rtc_alarm_twi_write_done);
    twi_send(0xd0, 2, TheBuffer);
  }
}

void rtc_alarm_twi_write_done(bool success)
{
  if (success) {
    /* Enable INT1 interrupt again */
    GICR |= (1U<<INT1);
  } else {
    hw_uart_write_string_P(PSTR("Error: failed clearing alarm flag in RTC.\r\n"));
  }
}

ISR(INT1_vect)
{
  TheIsrRequest = true;
  /* Disable the INT1 interrupt */
  GICR &= ~(1U<<INT1);
  GIFR |= (1U<<INTF1);
}
