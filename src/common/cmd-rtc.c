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
  hw_uart_write_uintd(date->month, true);
  hw_uart_write_string_P(PSTR("-"));
  hw_uart_write_uintd(date->day, true);
  hw_uart_write_string_P(PSTR(" ("));
  hw_uart_write_uintd(date->dayOfWeek, true);
  hw_uart_write_string_P(PSTR(")\r\n"));

  cmd_engine_start();
}
