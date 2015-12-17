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

#include "hw-rtc.h"
#include "hw-uart.h"
#include "mglobal.h"

static void cmd_engine_date_ready(bool success, const MDate *date);
static void cmd_engine_rtc_time_ready(bool success, const MTime *time);

void cmd_engine_rtc_help(void)
{
  hw_uart_write_string_P(PSTR("> rtc-rd - Read the current time\r\n"));
  hw_uart_write_string_P(PSTR("> rtc-wr - Update the current time\r\n"));
  hw_uart_write_string_P(PSTR("> date-rd - Read the current date\r\n"));
  hw_uart_write_string_P(PSTR("> date-wr - Update the current date\r\n"));
}

bool cmd_engine_rtc_command(const char *args, bool *startCmd)
{
  if (!strcmp_P(args, PSTR("rtc-rd"))) {
    *startCmd = false;
    mtime_get_time(cmd_engine_rtc_time_ready);
    return true;
  } else if (!strcmp_P(args, PSTR("date-rd"))) {
    *startCmd = false;
    mtime_get_date(cmd_engine_date_ready);
    return true;
  }

  return false;
}

void cmd_engine_rtc_time_ready(bool success, const MTime *time)
{
  if (!success) {
    hw_uart_write_string_P(PSTR("RTC communication failed.\r\n"));
    cmd_engine_start();
    return;
  }

  hw_uart_write_string_P(PSTR("Time: "));
  hw_uart_write_uintd(time->hours, true);
  hw_uart_write_string_P(PSTR(":"));
  hw_uart_write_uintd(time->minutes, true);
  hw_uart_write_string_P(PSTR(":"));
  hw_uart_write_uintd(time->seconds, true);
  hw_uart_write_string_P(PSTR("\r\n"));

  cmd_engine_start();
}

void cmd_engine_date_ready(bool success, const MDate *date)
{
  if (!success) {
    hw_uart_write_string_P(PSTR("RTC communication failed.\r\n"));
    cmd_engine_start();
    return;
  }

  hw_uart_write_string_P(PSTR("Date: "));
  hw_uart_write_uintd(date->year, true);
  hw_uart_write_string_P(PSTR("-"));
  hw_uart_write_string_P(mtime_get_month_name(date->month));
  hw_uart_write_string_P(PSTR("-"));
  hw_uart_write_uintd(date->day, true);
  hw_uart_write_string_P(PSTR(" ("));
  hw_uart_write_string_P(mtime_get_day_of_week_name(date->dayOfWeek));
  hw_uart_write_string_P(PSTR(")\r\n"));

  cmd_engine_start();
}
