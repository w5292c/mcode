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

/*typedef struct {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
} MTime;

typedef struct {
  uint8_t dayOfWeek;
  uint8_t day;
  uint8_t month;
  int16_t year;
} MDate;

typedef void (*mtime_time_ready)(bool success, const MTime *time);
typedef void (*mtime_date_ready)(bool success, const MDate *time);*/

/*#define MCODE_DS3231_BUFFER_LENGTH (4)*/

typedef enum {
  RtcStateNull = 0,
  RtcStateIdle,
  RtcStateReadTime,
  RtcStateReadDate,
  RtcStateSetAddressForTime,
} RtcState;

static uint8_t TheState;
static uint8_t TheBuffer[4];
static mtime_time_ready TheTimeCallback;

static void mtime_write_ready(bool success);
static void mtime_parse_time(const uint8_t *data);
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

void mtime_get_date(mtime_date_ready callback)
{
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
  const uint8_t hours = *data++;
  time.hours = 10*((hours>>4)&0x03u) + (0x0fu&hours);
  (*TheTimeCallback)(true, &time);
  TheState = RtcStateIdle;
}
