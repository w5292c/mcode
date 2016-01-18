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

#include "main.h"
#include "mtick.h"
#include "utils.h"
#include "system.h"
#include "mglobal.h"
#include "mstring.h"

#include "hw-twi.h"

#include <string.h>

static void cmd_engine_call(const char *args, bool *startCmd);
static bool cmd_engine_echo(const char *args, bool *startCmd);
static bool cmd_engine_sleep(const char *args, bool *startCmd);
static void cmd_engine_system_test(const char *args, bool *startCmd);

void cmd_engine_system_help(void)
{
  mprintstrln(PSTR("> ut - Show uptime"));
  mprintstrln(PSTR("> reboot - Initiate a system reboot"));
#ifdef MCODE_BOOTLOADER
  mprintstrln(PSTR("> bootloader - Reboot to bootloader mode"));
#endif /* MCODE_BOOTLOADER */
  mprintstrln(PSTR("> call <addr> - Call a program at <addr>"));
  mprintstrln(PSTR("> echo <string> - Echo <string> to console"));
  mprintstrln(PSTR("> sleep <msec> - Suspend execution for <msec> milli-seconds"));
  mprintstrln(PSTR("> test [<args>] - Usually empty placeholder for temparary experiments"));
}

bool cmd_engine_system_command(const char *args, bool *startCmd)
{
  if (!strncmp_P(args, PSTR("sleep"), 5)) {
    return cmd_engine_sleep(args + 5, startCmd);
  } else if (!strncmp_P(args, PSTR("echo"), 4)) {
    return cmd_engine_echo(args + 4, startCmd);
  } else if (!strcmp_P(args, PSTR("ut"))) {
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
  } else if (!strcmp_P(args, PSTR("reboot"))) {
    reboot();
    return true;
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
  } else if (!strcmp_P(args, PSTR("test"))) {
    cmd_engine_system_test(NULL, startCmd);
    return true;
  } else if (!strncmp_P(args, PSTR("test "), 5)) {
    cmd_engine_system_test(args + 5, startCmd);
    return true;
  } else if (!strncmp_P(args, PSTR("call "), 5)) {
    cmd_engine_call(args + 5, startCmd);
    return true;
  }
#ifdef __linux__
  else if (!strcmp_P(args, PSTR("quit")) || !strcmp_P(args, PSTR("exit"))) {
    main_request_exit();
    *startCmd = false;
    return true;
  }
#endif /* __linux__ */

  return false;
}

bool cmd_engine_echo(const char *args, bool *startCmd)
{
  if (*args) {
    ++args;
    mprintstr_R(args);
  }

  mprint(MStringNewLine);
  return true;
}

bool cmd_engine_sleep(const char *args, bool *startCmd)
{
  /* Get the 'mticks' argument */
  uint16_t mticks = 0;
  args = string_skip_whitespace(args);
  string_next_number(args, &mticks);
  if (!mticks) {
    /* Wrong 'mticks' argument */
    merror(MStringWrongArgument);
    return true;
  }

  mtick_sleep(mticks);
  return true;
}

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
  program();
}

void cmd_engine_system_test(const char *args, bool *startCmd)
{
  uint8_t buffer[18];
  memset(buffer + 2, 0xffu, 16);
  buffer[0] = 0x00u;
  buffer[1] = 0x20u;
  if (!twi_send_sync(0xaeu, 18, buffer)) {
    mprintstrln(PSTR("Error 0"));
    return;
  }

  buffer[0] = 0x00u;
  buffer[1] = 0x20u;
  if (!twi_send_sync(0xaeu, 2, buffer)) {
    mprintstrln(PSTR("Error 1"));
    return;
  }

  if (!twi_recv_sync(0xaeu, 16, buffer)) {
    mprintstrln(PSTR("Error 2"));
    return;
  }
  mprint_dump_buffer(16, buffer, true);
}
