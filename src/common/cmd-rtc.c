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
#include "mglobal.h"
#include "mstring.h"

#include <string.h>

typedef enum {
  CmdEngineRtcSetTimeTargetTime,
  CmdEngineRtcSetTimeTargetAlarm,
  CmdEngineRtcSetTimeTargetNewDayAlarm,
} CmdEngineRtcSetTimeTarget;

typedef enum {
  CmdEngineRtcErrorHw,
} CmdEngineRtcError;

static void cmd_engine_update_ready(bool success);
static const char *cmd_engine_rtc_error_string(uint8_t error);
static bool cmd_engine_set_date(const char *args, bool *startCmd);
static void cmd_engine_date_ready(bool success, const MDate *date);
static void cmd_engine_rtc_time_ready(bool success, const MTime *time);
static bool cmd_engine_set_time(const char *args, bool *startCmd, uint8_t target);

void cmd_engine_rtc_help(void)
{
  mprintstrln(PSTR("> rtc init - First time initialization for RTC"));
  mprintstrln(PSTR("> time - Read the current time"));
  mprintstrln(PSTR("> time-set <hour> <min> <sec> - Update the current time"));
  mprintstrln(PSTR("> date - Read the current date"));
  mprintstrln(PSTR("> date-set <year> <month> <day> <weekday> - Update the current date"));
  mprintstrln(PSTR("> alarm-set <hour> <min> <sec> - Update the current alarm"));
  mprintstrln(PSTR("> new-day-set <hour> <min> <sec> - Update the new-day alarm"));
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
    return cmd_engine_set_time(args + 8, startCmd, CmdEngineRtcSetTimeTargetTime);
  } else if (!strncmp_P(args, PSTR("date-set"), 8)) {
    return cmd_engine_set_date(args + 8, startCmd);
  } else if (!strncmp_P(args, PSTR("alarm-set"), 9)) {
    return cmd_engine_set_time(args + 9, startCmd, CmdEngineRtcSetTimeTargetAlarm);
  } else if (!strncmp_P(args, PSTR("new-day-set "), 12)) {
    return cmd_engine_set_time(args + 12, startCmd, CmdEngineRtcSetTimeTargetNewDayAlarm);
  } else if (!strcmp_P(args, PSTR("rtc init"))) {
    rtc_first_time_init();
    return true;
  }

  return false;
}

bool cmd_engine_set_time(const char *args, bool *startCmd, uint8_t target)
{
#ifdef MCODE_COMMAND_MODES
  if (CmdModeRoot != cmd_engine_get_mode()) {
    merror(MStringWrongMode);
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    merror(MStringWrongArgument);
    return true;
  }
  /* Get the 'hour' argument */
  uint16_t hour = -1;
  args = string_next_number(args, &hour);
  if (!args || hour > 23u) {
    /* Wrong 'hour' argument */
    merror(MStringWrongArgument);
    return true;
  }
  /* Parse <min> argument */
  args = string_skip_whitespace(args);
  uint16_t mins = -1;
  args = string_next_number(args, &mins);
  if (!args || mins > 59u) {
    /* Wrong 'min' argument */
    merror(MStringWrongArgument);
    return true;
  }
  /* Parse <seconds> argument */
  args = string_skip_whitespace(args);
  uint16_t secs = -1;
  args = string_next_number(args, &secs);
  if (args || secs > 59) {
    /* Wrong 'second' argument */
    merror(MStringWrongArgument);
    return true;
  }

  switch (target) {
  case CmdEngineRtcSetTimeTargetTime:
    mtime_set_time(hour, mins, secs, cmd_engine_update_ready);
    break;
  case CmdEngineRtcSetTimeTargetAlarm:
    mtime_set_alarm(hour, mins, secs, cmd_engine_update_ready);
    break;
  case CmdEngineRtcSetTimeTargetNewDayAlarm:
    mtime_set_new_day_alarm(hour, mins, secs, cmd_engine_update_ready);
    break;
  default:
    return false;
  }

  *startCmd = false;
  return true;
}

bool cmd_engine_set_date(const char *args, bool *startCmd)
{
#ifdef MCODE_COMMAND_MODES
  if (CmdModeRoot != cmd_engine_get_mode()) {
    merror(MStringWrongMode);
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

  /* Parse the 'year' argument */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    merror(MStringWrongArgument);
    return true;
  }
  uint16_t year = 0;
  args = string_next_number(args, &year);
  if (!args || year < 1900 || year > 2099) {
    /* Wrong 'year' argument */
    merror(MStringWrongArgument);
    return true;
  }
  /* Parse <month> argument */
  args = string_skip_whitespace(args);
  uint16_t month = 0;
  args = string_next_number(args, &month);
  if (!args || month < 1 || month > 12) {
    /* Wrong 'month' argument */
    merror(MStringWrongArgument);
    return true;
  }
  /* Parse <day> argument */
  args = string_skip_whitespace(args);
  uint16_t day = 0;
  args = string_next_number(args, &day);
  if (!args || day < 1 || day > 31) {
    /* Wrong 'day' argument */
    merror(MStringWrongArgument);
    return true;
  }
  /* Parse <week-day> argument */
  args = string_skip_whitespace(args);
  uint16_t weekday = 0;
  args = string_next_number(args, &weekday);
  if (args || weekday < 1 || weekday > 7) {
    /* Wrong 'week-day' argument */
    merror(MStringWrongArgument);
    return true;
  }

  *startCmd = false;
  mtime_set_date(year, month, day, weekday, cmd_engine_update_ready);
  return true;
}

void cmd_engine_update_ready(bool success)
{
  if (!success) {
    mprintstrln(cmd_engine_rtc_error_string(CmdEngineRtcErrorHw));
  }

  cmd_engine_start();
}

void cmd_engine_rtc_time_ready(bool success, const MTime *time)
{
  if (!success) {
    mprint(MStringInternalError);
    cmd_engine_start();
    return;
  }

  mprintstr(PSTR("Time: "));
  mprint_uintd(time->hours, 2);
  mputch(':');
  mprint_uintd(time->minutes, 2);
  mputch(':');
  mprint_uintd(time->seconds, 2);
  mprint(MStringNewLine);

  cmd_engine_start();
}

void cmd_engine_date_ready(bool success, const MDate *date)
{
  if (!success) {
    mprint(MStringInternalError);
    cmd_engine_start();
    return;
  }

  mprintstr(PSTR("Date: "));
  mprint_uintd(date->year, 4);
  mputch('-');
  mprintstr(mtime_get_month_name(date->month));
  mputch('-');
  mprint_uintd(date->day, 2);
  mprintstr(PSTR(" ("));
  mprintstr(mtime_get_day_of_week_name(date->dayOfWeek));
  mprintstrln(PSTR(")"));

  cmd_engine_start();
}

const char *cmd_engine_rtc_error_string(uint8_t error)
{
  switch (error) {
  case CmdEngineRtcErrorHw:
    return PSTR("RTC: communication failed.");
  default:
    return PSTR("RTC: unrecognized error.");
  }
}
