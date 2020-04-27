/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2020 Alexander Chumakov
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

#include "mcode-config.h"
#include "cmd-iface.h"

#include "main.h"
#include "mtick.h"
#include "utils.h"
#include "hw-wdt.h"
#include "system.h"
#include "mglobal.h"
#include "mparser.h"
#include "mstatus.h"
#include "mstring.h"
#include "scheduler.h"
#include "cmd-engine.h"

CMD_IMPL("ut", TheUt, "Show uptime", cmd_system_ut, NULL, 0);
CMD_IMPL("echo", TheEcho, "Echo input string", cmd_system_echo, NULL, 0);
CMD_IMPL("expr", TheExpr, "Resolve/print input string", cmd_system_expr, NULL, 0);
CMD_IMPL("errno", TheErrno, "Print/reset error code", cmd_system_errno, NULL, 0);
CMD_IMPL("sleep", TheSleep, "Sleep for <milli-seconds>", cmd_system_sleep, NULL, 0);
CMD_IMPL("reboot", TheReboot, "Reboot system", cmd_system_reboot, NULL, 0);
CMD_IMPL("poweroff", ThePoweroff, "Power off system", cmd_system_poweroff, NULL, 0);
#ifdef MCODE_ID
CMD_IMPL("uid", TheId, "Show device ID", cmd_system_id, NULL, 0);
#endif /* MCODE_ID */

bool cmd_system_ut(const TCmdData *data, const char *args,
                   size_t args_len, bool *start_cmd)
{
#ifdef MCODE_WDT
  mprintstr(PSTR("Reset reason: 0x"));
  mprint_uint8(wdt_reset_reason(), false);
  mprint(MStringNewLine);
#endif /* MCODE_WDT */
  const uint64_t tickCount = mtick_count();
  uint64_t count = tickCount;
  const uint32_t milliSeconds = count%1000;
  count = count/1000;
  const uint32_t seconds = count%60;
  count = count/60;
  const uint32_t minutes = count%60;
  count = count/60;
  const uint32_t hours = count%24;
  count = count/24;
  const uint32_t days = count;
  mprintstr(PSTR("Uptime: 0x"));
  mprint_uint64(tickCount, true);
  mprintstr(PSTR("; days: "));
  mprint_uintd(days, 0);
  mprintstr(PSTR(", hours: "));
  mprint_uintd(hours, 0);
  mprintstr(PSTR(", minutes: "));
  mprint_uintd(minutes, 0);
  mprintstr(PSTR(", seconds: "));
  mprint_uintd(seconds, 0);
  mprintstr(PSTR(", milli-seconds: "));
  mprint_uintd(milliSeconds, 0);
  mprint(MStringNewLine);
  return true;
}

bool cmd_system_echo(const TCmdData *data, const char *args,
                     size_t args_len, bool *start_cmd)
{
  mprintbytes_R(args, args_len);
  mprint(MStringNewLine);
  return true;
}

bool cmd_system_expr(const TCmdData *data, const char *args,
                     size_t args_len, bool *start_cmd)
{
  mprintexpr(args, -1);
  mprint(MStringNewLine);
  return true;
}

bool cmd_system_errno(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  mprintstr(PSTR("errno: "));
  mprint_uintd(mcode_errno(), 0);
  mcode_errno_set(ESuccess);
  mprint(MStringNewLine);
  return true;
}

bool cmd_system_sleep(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  uint32_t value;
  TokenType type;
  const char *token = NULL;

  type = next_token(&args, &args_len, &token, &value);
  if (TokenInt != type || value > 10000) {
    /* Wrong argument or requested to sleep for too long
     * (more than 10 seconds, need to check if this is ok) */
    mcode_errno_set(EArgument);
    return true;
  }

  /* Request to sleep synchronously */
  mtick_sleep(value);

  return true;
}

bool cmd_system_reboot(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  reboot();
  *start_cmd = false;
  return true;
}

bool cmd_system_poweroff(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
#ifndef __linux__
  scheduler_stop();
#else /* !__linux__ */
  main_request_exit();
#endif /* !__linux__ */

  *start_cmd = false;
  return true;
}

#ifdef MCODE_ID
bool cmd_system_id(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  mprint_uint32(mcode_id(0), false);
  mprint_uint32(mcode_id(1), false);
  mprint_uint32(mcode_id(2), false);
  mprint(MStringNewLine);

  return true;
}
#endif /* MCODE_ID */
