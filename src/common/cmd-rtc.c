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

#include "utils.h"
#include "hw-rtc.h"
#include "hw-uart.h"
#include "mglobal.h"

typedef enum {
  CmdEngineRtcErrorArgs,
  CmdEngineRtcErrorHw,
} CmdEngineRtcError;

static void cmd_engine_update_ready(bool success);
static const char *cmd_engine_rtc_error_string(uint8_t error);
static bool cmd_engine_set_time(const char *args, bool *startCmd);
static bool cmd_engine_set_date(const char *args, bool *startCmd);
static void cmd_engine_date_ready(bool success, const MDate *date);
static void cmd_engine_rtc_time_ready(bool success, const MTime *time);

void cmd_engine_rtc_help(void)
{
  hw_uart_write_string_P(PSTR("> time - Read the current time\r\n"));
  hw_uart_write_string_P(PSTR("> time-set <hour> <min> <sec> - Update the current time\r\n"));
  hw_uart_write_string_P(PSTR("> date - Read the current date\r\n"));
  hw_uart_write_string_P(PSTR("> date-set <year> <month> <day> <weekday> - Update the current date\r\n"));
}

bool cmd_engine_rtc_command(const char *args, bool *startCmd)
{
  if (!strcmp_P(args, PSTR("time"))) {
    *startCmd = false;
    mtime_get_time(cmd_engine_rtc_time_ready);
    return true;
  } else if (!strcmp_P(args, PSTR("date"))) {
    *startCmd = false;
    mtime_get_date(cmd_engine_date_ready);
    return true;
  } else if (!strncmp_P(args, PSTR("time-set"), 8)) {
    return cmd_engine_set_time(args + 8, startCmd);
  } else if (!strncmp_P(args, PSTR("date-set"), 8)) {
    return cmd_engine_set_date(args + 8, startCmd);
  }

  return false;
}

bool cmd_engine_set_time(const char *args, bool *startCmd)
{
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  /* Get the 'hour' argument */
  int hour = -1;
  args = string_next_number(args, &hour);
  if (!args || !*args || hour > 23 || hour < 0) {
    /* Wrong 'hour' argument */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  /* Parse <min> argument */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  int mins = -1;
  args = string_next_number(args, &mins);
  if (!args || !*args || mins > 59 || mins < 0) {
    /* Wrong 'min' argument */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  /* Parse <seconds> argument */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  int secs = -1;
  args = string_next_number(args, &secs);
  if (!args || *args || secs > 59 || secs < 0) {
    /* Wrong 'second' argument */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }

  *startCmd = false;
  mtime_set_time(hour, mins, secs, cmd_engine_update_ready);
  return true;
}

bool cmd_engine_set_date(const char *args, bool *startCmd)
{
  /* Parse the 'year' argument */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  int year = -1;
  args = string_next_number(args, &year);
  if (!args || !*args || year < 1900 || year > 2099) {
    /* Wrong 'year' argument */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  /* Parse <month> argument */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  int month = -1;
  args = string_next_number(args, &month);
  if (!args || !*args || month < 1 || month > 12) {
    /* Wrong 'month' argument */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  /* Parse <day> argument */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  int day = -1;
  args = string_next_number(args, &day);
  if (!args || !*args || day < 1 || day > 31) {
    /* Wrong 'day' argument */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  /* Parse <week-day> argument */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }
  int weekday = -1;
  args = string_next_number(args, &weekday);
  if (!args || *args || weekday < 1 || weekday > 7) {
    /* Wrong 'week-day' argument */
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorArgs));
    return true;
  }

  *startCmd = false;
  mtime_set_date(year, month, day, weekday, cmd_engine_update_ready);
  return true;
}

void cmd_engine_update_ready(bool success)
{
  if (!success) {
    hw_uart_write_string_P(cmd_engine_rtc_error_string(CmdEngineRtcErrorHw));
  }

  cmd_engine_start();
}

void cmd_engine_rtc_time_ready(bool success, const MTime *time)
{
  if (!success) {
    hw_uart_write_string_P(PSTR("RTC communication failed.\r\n"));
    cmd_engine_start();
    return;
  }

  hw_uart_write_string_P(PSTR("Time: "));
  hw_uart_write_uintd(time->hours, 2);
  hw_uart_write_string_P(PSTR(":"));
  hw_uart_write_uintd(time->minutes, 2);
  hw_uart_write_string_P(PSTR(":"));
  hw_uart_write_uintd(time->seconds, 2);
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
  hw_uart_write_uintd(date->year, 4);
  hw_uart_write_string_P(PSTR("-"));
  hw_uart_write_string_P(mtime_get_month_name(date->month));
  hw_uart_write_string_P(PSTR("-"));
  hw_uart_write_uintd(date->day, 2);
  hw_uart_write_string_P(PSTR(" ("));
  hw_uart_write_string_P(mtime_get_day_of_week_name(date->dayOfWeek));
  hw_uart_write_string_P(PSTR(")\r\n"));

  cmd_engine_start();
}

const char *cmd_engine_rtc_error_string(uint8_t error)
{
  switch (error) {
  case CmdEngineRtcErrorArgs:
    return PSTR("RTC: wrong arguments.\r\n");
  case CmdEngineRtcErrorHw:
    return PSTR("RTC: communication failed.\r\n");
  default:
    return PSTR("RTC: unrecognized error.\r\n");
  }
}
