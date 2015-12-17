#include "cmd-engine.h"

#include "hw-rtc.h"
#include "hw-uart.h"
#include "mglobal.h"

static void cmd_engine_rtc_time_ready(bool success, const MTime *time);

void cmd_engine_rtc_help(void)
{
  hw_uart_write_string_P(PSTR("> rtc-rd - Read the current time\r\n"));
  hw_uart_write_string_P(PSTR("> rtc-wr - Update the current time\r\n"));
}

bool cmd_engine_rtc_read(const char *args)
{
  if (strcmp_P(args, PSTR("rtc-rd"))) {
    return false;
  }

  mtime_get_time(cmd_engine_rtc_time_ready);
  return true;
}

bool cmd_engine_rtc_write(const char *args)
{
  if (strncmp_P(args, PSTR("rtc-wr "), 7)) {
    return false;
  }

  args += 7;

  return false;
}

void cmd_engine_rtc_time_ready(bool success, const MTime *time)
{
  if (!success) {
    hw_uart_write_string_P(PSTR("TWI write failed.\r\n"));
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
