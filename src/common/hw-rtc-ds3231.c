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
#include "mglobal.h"

typedef enum {
  RtcStateNull = 0,
  RtcStateIdle,
  RtcStateReadTime,
  RtcStateReadDate,
  RtcStateSetAddressForTime,
  RtcStateSetAddressForDate,
  RtcStateSetTime,
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
  twi_set_read_callback(mtime_read_ready);
  twi_set_write_callback(mtime_write_ready);
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
    twi_send(0xd0u, 1, TheBuffer);
  } else {
    /* Already in progress */
    (*callback)(false, NULL);
  }
}

void mtime_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback)
{
  if (RtcStateIdle == TheState) {
    if (hours > 23) {
      hw_uart_write_string_P(PSTR("Error: set time: wrong hour parameter"));
      (*callback)(false);
      return;
    }
    if (minutes > 59) {
      hw_uart_write_string_P(PSTR("Error: set time: wrong minutes parameter"));
      (*callback)(false);
      return;
    }
    if (seconds > 59) {
      hw_uart_write_string_P(PSTR("Error: set time: wrong seconds parameter"));
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
    twi_send(0xd0u, 4, TheBuffer);
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
    twi_send(0xd0u, 1, TheBuffer);
  } else {
    /* Already in progress */
    (*callback)(false, NULL);
  }
}

void mtime_set_date(uint8_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, mcode_done callback)
{
  if (RtcStateIdle == TheState) {
    if (year < 1900 || year > 2099) {
      hw_uart_write_string_P(PSTR("Error: set date: wrong year parameter"));
      (*callback)(false);
      return;
    }
    if (month < 1 || month > 31) {
      hw_uart_write_string_P(PSTR("Error: set date: wrong month parameter"));
      (*callback)(false);
      return;
    }
    if (day < 1 || day > 31) {
      hw_uart_write_string_P(PSTR("Error: set date: wrong day parameter"));
      (*callback)(false);
      return;
    }
    if (dayOfWeek < 1 || dayOfWeek > 7) {
      hw_uart_write_string_P(PSTR("Error: set date: wrong day-of-week parameter"));
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
    TheState = RtcStateSetTime;
    TheWriteCallback = callback;
    twi_send(0xd0u, 5, TheBuffer);
  } else {
    (*callback)(false);
  }
}

void mtime_write_ready(bool success)
{
  if (!success) {
    hw_uart_write_string_P(PSTR("RTC: error: failed writing to the TWI interface\r\n"));
    return;
  }

  switch (TheState) {
  case RtcStateSetAddressForTime:
    TheState = RtcStateReadTime;
    twi_recv(0xd0u, 3);
    break;
  case RtcStateSetAddressForDate:
    TheState = RtcStateReadDate;
    twi_recv(0xd0u, 4);
    break;
  case RtcStateSetTime:
    (*TheWriteCallback)(true);
    TheState = RtcStateIdle;
    break;
  default:
    hw_uart_write_string_P(PSTR("RTC: error: wrong state\r\n"));
    break;
  }
}

void mtime_read_ready(bool success, uint8_t length, const uint8_t *data)
{
  if (!success) {
    hw_uart_write_string_P(PSTR("RTC: error: failed reading the TWI interface\r\n"));
    return;
  }

  switch (TheState) {
  case RtcStateReadTime:
    if (3 == length) {
      mtime_parse_time(data);
    } else {
      hw_uart_write_string_P(PSTR("RTC: error: wrong data length\r\n"));
    }
    break;
  case RtcStateReadDate:
    if (4 == length) {
      mtime_parse_date(data);
    } else {
      hw_uart_write_string_P(PSTR("RTC: error: wrong data length\r\n"));
    }
    break;
  default:
    hw_uart_write_string_P(PSTR("RTC: error: wrong state\r\n"));
    break;
  }
}

void mtime_parse_time(const uint8_t *data)
{
  if (!TheTimeCallback) {
    hw_uart_write_string_P(PSTR("RTC: error: no callback\r\n"));
    return;
  }

  MTime time;
  const uint8_t seconds = *data++;
  time.seconds = 10*((seconds>>4)&0x07u) + (0x0fu&seconds);
  const uint8_t minutes = *data++;
  time.minutes = 10*((minutes>>4)&0x07u) + (0x0fu&minutes);
  const uint8_t hours = *data;
  time.hours = 10*((hours>>4)&0x03u) + (0x0fu&hours);
  (*TheTimeCallback)(true, &time);
  TheState = RtcStateIdle;
}

void mtime_parse_date(const uint8_t *data)
{
  if (!TheDateCallback) {
    hw_uart_write_string_P(PSTR("RTC: error: no callback\r\n"));
    return;
  }

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

  (*TheDateCallback)(true, &date);
  TheState = RtcStateIdle;
}
