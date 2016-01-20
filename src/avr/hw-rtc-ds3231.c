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
#include "mstring.h"
#include "hw-sound.h"
#include "scheduler.h"
#include "cmd-engine.h"

#include <avr/interrupt.h>

static bool volatile TheIsrRequest;

static void rtc_alarm_tick(void);
static bool rtc_alarm_check_status(uint8_t status);

void rtc_alarm_init(void)
{
  /* Configure INT1 pin (PD3) as input */
  DDRD &= ~_BV(DDD3);
  /* Enable pull-up resistor for PD3 */
  PORTD |= _BV(PD3);
  /* Generate an interrupt on low level of INT1 pin (PD3) */
  MCUCR |= (0U<<ISC10)|(0U<<ISC11);
  /* Enable external interrupt request INT1 (PD3) */
  GICR |= _BV(INT1);
  mcode_scheduler_add(rtc_alarm_tick);
}

void rtc_alarm_deinit(void)
{
}

void rtc_alarm_tick(void)
{
  if (TheIsrRequest) {
    TheIsrRequest = false;
    /* Now, check which alarm expired */
    uint8_t buffer;
    buffer = 0x0fu;
    /* Write the address of the status register */
    if (!twi_send_sync(0xd0u, 1, &buffer)) {
      /* Cannot write, try next tick */
      TheIsrRequest = true;
      return;
    }
    /* Read the status */
    if (!twi_recv_sync(0xd0u, 1, &buffer)) {
      /* Cannot read, try next tick */
      TheIsrRequest = true;
      return;
    }
    /* Check/handle the status */
    if (!rtc_alarm_check_status(buffer)) {
      /* Cannot handle status, try next tick */
      TheIsrRequest = true;
      return;
    }
  }
}

bool rtc_alarm_check_status(uint8_t status)
{
  /* First, check the alarm-1 */
  if (status & 0x01u) {
    mprintstrln(PSTR("Alarm1: triggered."));
#ifdef MCODE_SOUND
    sound_play_note(0x49u, 500);
    sound_play_note(0x45u, 500);
    sound_play_note(0x40u, 500);
    sound_play_note(0x44u, 500);
#endif /* MCODE_SOUND */
  }
  /* Now, check the alarm-2 (new-day alarm) */
  if (status & 0x02u) {
#ifdef MCODE_TV
    mprintstrln(PSTR("Alarm2: triggered."));
    cmd_engine_tv_new_day();
#endif /* MCODE_TV */
  }

  /* now, we are ready to clear the alarm(s) flags */
  if (0u != (status & 0x03u)) {
    uint8_t buffer[2];
    buffer[0] = 0x0f;
    buffer[1] = 0x88;
    if (!twi_send_sync(0xd0u, 2, buffer)) {
      /* Cannot clear the status */
      return false;
    }
  }

  /* Enable INT1 interrupt again */
  GICR |= _BV(INT1);
  return true;
}

ISR(INT1_vect)
{
  TheIsrRequest = true;
  /* Disable the INT1 interrupt */
  GICR &= ~_BV(INT1);
  GIFR |= _BV(INTF1);
}
