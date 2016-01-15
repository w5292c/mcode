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
#include "mglobal.h"
#include "mstring.h"

typedef enum {
  RtcStateNull = 0,
  RtcStateIdle,
  RtcStateReadTime,
  RtcStateReadDate,
  RtcStateSetAddressForTime,
  RtcStateSetAddressForDate,
  RtcStateSetTime,
  RtcStateSetDate,
  RtcStateSetAlarm,
  RtcStateSetNewDayAlarm,
} RtcState;

static uint8_t TheState;
static uint8_t TheBuffer[5];
static mcode_done TheWriteCallback;
static mtime_time_ready TheTimeCallback;
static mtime_date_ready TheDateCallback;

static void mtime_write_ready(bool success);
static void mtime_parse_time(const uint8_t *data);
static void mtime_parse_date(const uint8_t *data);
static void mtime_read_ready(bool success, uint8_t length, const uint8_t *data);

void mtime_init(void)
{
  TheState = RtcStateIdle;
}

void mtime_deinit(void)
{
}

void mtime_get_time(mtime_time_ready callback)
{
  if (RtcStateIdle == TheState) {
    TheBuffer[0] = 0;
    TheTimeCallback = callback;
    TheState = RtcStateSetAddressForTime;
    twi_send(0xd0u, 1, TheBuffer, mtime_write_ready);
  } else {
    /* Already in progress */
    (*callback)(false, NULL);
  }
}

void mtime_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback)
{
  if (RtcStateIdle == TheState) {
    if (hours > 23) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (minutes > 59) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (seconds > 59) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    /* Set address: 0x00 */
    TheBuffer[0] = 0;
    /* Set seconds */
    TheBuffer[1] = (seconds % 10) | ((seconds/10)*0x10);
    /* Set minutes */
    TheBuffer[2] = (minutes % 10) | ((minutes/10)*0x10);
    /* Set hours */
    TheBuffer[3] = (hours % 10) | ((hours/10)*0x10);
    TheState = RtcStateSetTime;
    TheWriteCallback = callback;
    twi_send(0xd0u, 4, TheBuffer, mtime_write_ready);
  } else {
    (*callback)(false);
  }
}

void mtime_get_date(mtime_date_ready callback)
{
  if (RtcStateIdle == TheState) {
    TheBuffer[0] = 3;
    TheDateCallback = callback;
    TheState = RtcStateSetAddressForDate;
    twi_send(0xd0u, 1, TheBuffer, mtime_write_ready);
  } else {
    /* Already in progress */
    (*callback)(false, NULL);
  }
}

void mtime_set_date(int16_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, mcode_done callback)
{
  if (RtcStateIdle == TheState) {
    if (year < 1900 || year > 2099) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (month < 1 || month > 31) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (day < 1 || day > 31) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (dayOfWeek < 1 || dayOfWeek > 7) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    /* Set address: 0x03 */
    TheBuffer[0] = 3;
    /* Set day-of-week */
    TheBuffer[1] = dayOfWeek;
    /* Set day */
    TheBuffer[2] = (day % 10) | ((day/10)*0x10);
    /* Set month/century */
    TheBuffer[3] = (month % 10) | ((month/10)*0x10) | ((year < 2000) ? 0x80u : 0x00u);
    TheBuffer[4] = (year % 10) | (((year/10) % 10)*0x10);
    TheState = RtcStateSetDate;
    TheWriteCallback = callback;
    twi_send(0xd0u, 5, TheBuffer, mtime_write_ready);
  } else {
    (*callback)(false);
  }
}

void mtime_set_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback)
{
  if (RtcStateIdle == TheState) {
    if (hours > 23) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (minutes > 59) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (seconds > 59) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    /* Set address: 0x07 (Alarm1) */
    TheBuffer[0] = 0x07;
    /* Set seconds */
    TheBuffer[1] = (seconds % 10) | ((seconds/10)*0x10);
    /* Set minutes */
    TheBuffer[2] = (minutes % 10) | ((minutes/10)*0x10);
    /* Set hours */
    TheBuffer[3] = (hours % 10) | ((hours/10)*0x10);
    /* No day/date match */
    TheBuffer[4] = 0x80;
    TheState = RtcStateSetAlarm;
    TheWriteCallback = callback;
    twi_send(0xd0u, 5, TheBuffer, mtime_write_ready);
  } else {
    (*callback)(false);
  }
}

void mtime_set_new_day_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback)
{
  if (RtcStateIdle == TheState) {
    if (hours > 23) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    if (minutes > 59) {
      mprint(MStringWrongArgument);
      (*callback)(false);
      return;
    }
    /* Set address: 0x07 (Alarm1) */
    TheBuffer[0] = 0x0b;
    /* Set minutes */
    TheBuffer[1] = (minutes % 10) | ((minutes/10)*0x10);
    /* Set hours */
    TheBuffer[2] = (hours % 10) | ((hours/10)*0x10);
    /* No day/date match */
    TheBuffer[3] = 0x80;
    TheState = RtcStateSetNewDayAlarm;
    TheWriteCallback = callback;
    twi_send(0xd0u, 4, TheBuffer, mtime_write_ready);
  } else {
    (*callback)(false);
  }
}

void mtime_write_ready(bool success)
{
  if (!success) {
    merror(MStringInternalError);
    TheState = RtcStateIdle;
    if (TheWriteCallback) {
      (*TheWriteCallback)(false);
    }
    return;
  }

  switch (TheState) {
  case RtcStateSetAddressForTime:
    TheState = RtcStateReadTime;
    twi_recv(0xd0u, 3, mtime_read_ready);
    break;
  case RtcStateSetAddressForDate:
    TheState = RtcStateReadDate;
    twi_recv(0xd0u, 4, mtime_read_ready);
    break;
  case RtcStateSetTime:
  case RtcStateSetDate:
  case RtcStateSetAlarm:
  case RtcStateSetNewDayAlarm:
    TheState = RtcStateIdle;
    if (TheWriteCallback) {
      (*TheWriteCallback)(true);
    }
    break;
  default:
    merror(MStringInternalError);
    break;
  }
}

void mtime_read_ready(bool success, uint8_t length, const uint8_t *data)
{
  if (!success) {
    merror(MStringInternalError);
    TheState = RtcStateIdle;
    if (TheDateCallback) {
      (*TheDateCallback)(false, NULL);
    }
    return;
  }

  switch (TheState) {
  case RtcStateReadTime:
    if (3 == length) {
      mtime_parse_time(data);
    } else {
      merror(MStringInternalError);
      TheState = RtcStateIdle;
      if (TheDateCallback) {
        (*TheDateCallback)(false, NULL);
      }
    }
    break;
  case RtcStateReadDate:
    if (4 == length) {
      mtime_parse_date(data);
    } else {
      merror(MStringInternalError);
      TheState = RtcStateIdle;
      if (TheDateCallback) {
        (*TheDateCallback)(false, NULL);
      }
    }
    break;
  default:
    merror(MStringInternalError);
    TheState = RtcStateIdle;
    if (TheDateCallback) {
      (*TheDateCallback)(false, NULL);
    }
    break;
  }
}

void mtime_parse_time(const uint8_t *data)
{
  MTime time;
  const uint8_t seconds = *data++;
  time.seconds = 10*((seconds>>4)&0x07u) + (0x0fu&seconds);
  const uint8_t minutes = *data++;
  time.minutes = 10*((minutes>>4)&0x07u) + (0x0fu&minutes);
  const uint8_t hours = *data;
  time.hours = 10*((hours>>4)&0x03u) + (0x0fu&hours);
  TheState = RtcStateIdle;
  if (TheTimeCallback) {
    (*TheTimeCallback)(true, &time);
  }
}

void mtime_parse_date(const uint8_t *data)
{
  MDate date;
  const uint8_t dayOfWeek = *data++;
  date.dayOfWeek = dayOfWeek;
  const uint8_t day = *data++;
  date.day = 10*((day>>4)&0x03u) + (0x0fu&day);
  const uint8_t month = *data++;
  date.month = 10*((month>>4)&0x01u) + (0x0fu&month);
  const uint8_t year = *data;
  date.year = 10*((year>>4)&0x0fu) + (0x0fu&year);
  if (0x80u & month) {
    date.year += 1900;
  } else {
    date.year += 2000;
  }

  TheState = RtcStateIdle;
  if (TheDateCallback) {
    (*TheDateCallback)(true, &date);
  }
}
