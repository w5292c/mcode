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

#include "cmd-iface.h"

#include "main.h"
#include "hw-ir.h"
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
#include "mcode-config.h"

#include <string.h>

static bool cmd_system_ut(const TCmdData *data, const char *args,
                          size_t args_len, bool *start_cmd);
static bool cmd_system_echo(const TCmdData *data, const char *args,
                            size_t args_len, bool *start_cmd);
static bool cmd_system_expr(const TCmdData *data, const char *args,
                            size_t args_len, bool *start_cmd);
static bool cmd_system_errno(const TCmdData *data, const char *args,
                             size_t args_len, bool *start_cmd);
static bool cmd_system_sleep(const TCmdData *data, const char *args,
                             size_t args_len, bool *start_cmd);
static bool cmd_system_reboot(const TCmdData *data, const char *args,
                              size_t args_len, bool *start_cmd);
static bool cmd_system_poweroff(const TCmdData *data, const char *args,
                                size_t args_len, bool *start_cmd);

static const char TheSystemUtBase[] PROGMEM = ("ut");
static const char TheSystemEchoBase[] PROGMEM = ("echo");
static const char TheSystemExprBase[] PROGMEM = ("expr");
static const char TheSystemErrnoBase[] PROGMEM = ("errno");
static const char TheSystemSleepBase[] PROGMEM = ("sleep");
static const char TheSystemRebootBase[] PROGMEM = ("reboot");
static const char TheSystemPoweroffBase[] PROGMEM = ("poweroff");
static const char TheSystemUtHelp[] PROGMEM = ("Show uptime");
static const char TheSystemRebootHelp[] PROGMEM = ("Reboot system");
static const char TheSystemEchoHelp[] PROGMEM = ("Echo input string");
static const char TheSystemPoweroffHelp[] PROGMEM = ("Power off system");
static const char TheSystemSleepHelp[] PROGMEM = ("Sleep for milli-seconds");
static const char TheSystemExprHelp[] PROGMEM = ("Resolve/print input string");
static const char TheSystemErrnoHelp[] PROGMEM = ("Show and reset error code for last command");

CMD_ENTRY(TheSystemUtBase, TheCmdUt, TheSystemUtHelp, &cmd_system_ut, NULL, 0);
CMD_ENTRY(TheSystemEchoBase, TheCmdEcho, TheSystemEchoHelp, &cmd_system_echo, NULL, 0);
CMD_ENTRY(TheSystemExprBase, TheCmdExpr, TheSystemExprHelp, &cmd_system_expr, NULL, 0);
CMD_ENTRY(TheSystemErrnoBase, TheCmdErrno, TheSystemErrnoHelp, &cmd_system_errno, NULL, 0);
CMD_ENTRY(TheSystemSleepBase, TheCmdSleep, TheSystemSleepHelp, &cmd_system_sleep, NULL, 0);
CMD_ENTRY(TheSystemRebootBase, TheCmdReboot, TheSystemRebootHelp, &cmd_system_reboot, NULL, 0);
CMD_ENTRY(TheSystemPoweroffBase, TheCmdPoweroff,
          TheSystemPoweroffHelp, &cmd_system_poweroff, NULL, 0);

#ifdef __AVR__
/** @todo move to AVR-specific code */
static void cmd_engine_call(const char *args, bool *startCmd);
#endif /* __AVR__ */

void cmd_engine_system_help(void)
{
#ifdef MCODE_BOOTLOADER
  mprintstrln(PSTR("> bootloader - Reboot to bootloader mode"));
#endif /* MCODE_BOOTLOADER */
#ifdef __AVR__
  mprintstrln(PSTR("> call <addr> - Call a program at <addr>"));
#endif /* __AVR__ */
}

bool cmd_engine_system_command(const char *args, bool *startCmd)
{
  if (false) {
#ifdef MCODE_BOOTLOADER
  } else if (!strcmp_P(args, PSTR("bootloader"))) {
#ifdef MCODE_COMMAND_MODES
    if (CmdModeRoot != cmd_engine_get_mode()) {
      merror(MStringWrongMode);
      return true;
    }
#endif /* MCODE_COMMAND_MODES */
    bootloader();
    return true;
#endif /* MCODE_BOOTLOADER */
#ifdef __AVR__
  } else if (!strncmp_P(args, PSTR("call "), 5)) {
    cmd_engine_call(args + 5, startCmd);
    return true;
#endif /* __AVR__ */
  }

  return false;
}

#ifdef __AVR__
void cmd_engine_call(const char *args, bool *startCmd)
{
  uint16_t address = 0x0000u;
  args = string_skip_whitespace(args);
  if (!args) {
    merror(MStringWrongArgument);
    return;
  }
  args = string_next_number(args, &address);
  if (string_skip_whitespace(args)) {
    merror(MStringWrongArgument);
    return;
  }

#ifdef MCODE_COMMAND_MODES
  /* Make bootloader address available in non-root mode */
  if ((CmdModeRoot != cmd_engine_get_mode()) &&
      (CmdModeUser != cmd_engine_get_mode() || 0x7c00u != address)) {
    merror(MStringWrongMode);
    return;
  }
#endif /* MCODE_COMMAND_MODES */

  mprintstr(PSTR("Calling program at address: 0x"));
  mprint_uint64(address, true);
  mprint(MStringNewLine);

  const mcode_tick program = (mcode_tick)address;
#ifdef MCODE_WDT
  wdt_stop();
#endif /* MCODE_WDT */
  program();
#ifdef MCODE_WDT
  wdt_start();
#endif /* MCODE_WDT */
}
#endif /* __AVR__ */

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
  mprintexpr(args);
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
