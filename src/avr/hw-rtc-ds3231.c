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

typedef enum {
  RtcAlarmStateNull = 0,
  RtcAlarmStateIdle,
  RtcAlarmStateSetStatusAddress,
  RtcAlarmStateReadStatus,
  RtcAlarmStateClearFlag,
} RtcAlarmState;

static uint8_t TheState;
static bool volatile TheIsrRequest;

static void rtc_alarm_tick(void);
static void rtc_alarm_check_status(uint8_t status);
static void rtc_alarm_twi_write_done(bool success);
static void rtc_alarm_read_ready(bool success, uint8_t length, const uint8_t *data);

void rtc_alarm_init(void)
{
  /* Configure INT1 pin (PD3) as input */
  DDRD &= ~(1U << DDD3);
  /* Enable pull-up resistor for PD3 */
  PORTD |= (1U << PD3);
  MCUCR |= (0U<<ISC00)|(0U<<ISC01);
  GICR |= (1U<<INT1);
  TheState = RtcAlarmStateIdle;
  mcode_scheduler_add(rtc_alarm_tick);
}

void rtc_alarm_deinit(void)
{
}

static uint8_t TheBuffer[2];
void rtc_alarm_tick(void)
{
  if (TheIsrRequest) {
    /* Now, check which alarm expired */
    TheBuffer[0] = 0x0f;
    twi_send(0xd0, 1, TheBuffer, rtc_alarm_twi_write_done);
    TheState = RtcAlarmStateSetStatusAddress;
    TheIsrRequest = false;
  }
}

void rtc_alarm_twi_write_done(bool success)
{
  if (!success) {
    TheState = RtcAlarmStateIdle;
    mprintstrln(PSTR("Error: failed clearing alarm flag in RTC."));
    return;
  }

  switch (TheState) {
  case RtcAlarmStateSetStatusAddress:
    twi_recv(0xd0, 1, rtc_alarm_read_ready);
    TheState = RtcAlarmStateReadStatus;
    break;
  case RtcAlarmStateClearFlag:
    /* Enable INT1 interrupt again */
    GICR |= (1U<<INT1);
    break;
  default:
    mprintstrln(PSTR("Error: wrong state in alarm management."));
    break;
  }
}

void rtc_alarm_read_ready(bool success, uint8_t length, const uint8_t *data)
{
  if (!success) {
    TheState = RtcAlarmStateIdle;
    mprintstrln(PSTR("Error: failed reading TWI for RTC alarm management."));
    return;
  }

  switch (TheState) {
  case RtcAlarmStateReadStatus:
    if (1 == length) {
      rtc_alarm_check_status(*data);
    } else {
      mprintstrln(PSTR("Error: wrong read package."));
      TheState = RtcAlarmStateIdle;
    }
    break;
  default:
    mprintstrln(PSTR("Error: wrong state in alarm management."));
    TheState = RtcAlarmStateIdle;
    break;
  }
}

void rtc_alarm_check_status(uint8_t status)
{
  /* First, check the alarm-1 */
  if (status & 0x01u) {
    mprintstrln(PSTR("Alarm1: triggered."));
    cmd_engine_start();
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
  TheState = RtcAlarmStateClearFlag;
  TheBuffer[0] = 0x0f;
  TheBuffer[1] = 0x88;
  twi_send(0xd0, 2, TheBuffer, rtc_alarm_twi_write_done);
}

ISR(INT1_vect)
{
  TheIsrRequest = true;
  /* Disable the INT1 interrupt */
  GICR &= ~(1U<<INT1);
  GIFR |= (1U<<INTF1);
}
