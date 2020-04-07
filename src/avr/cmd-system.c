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

#include "utils.h"
#include "hw-wdt.h"
#include "system.h"
#include "mglobal.h"
#include "mstatus.h"
#include "cmd-engine.h"
#include "mcode-config.h"

static bool cmd_system_call(const TCmdData *data, const char *args,
                            size_t args_len, bool *start_cmd);
static const char TheCallBase[] PROGMEM = ("call");
static const char TheCallHelp[] PROGMEM = ("Call code at <address>");
CMD_ENTRY(TheCallBase, TheCmdCall, TheCallHelp, &cmd_system_call, NULL, 0);

#ifdef MCODE_BOOTLOADER
static bool cmd_system_bootloader(const TCmdData *data, const char *args,
                                  size_t args_len, bool *start_cmd);
static const char TheBootloaderBase[] PROGMEM = ("bootloader");
static const char TheBootloaderHelp[] PROGMEM = ("Enter bootloader mode");
CMD_ENTRY(TheBootloaderBase, TheCmdBootloader, TheBootloaderHelp, &cmd_system_bootloader, NULL, 0);
#endif /* MCODE_BOOTLOADER */

bool cmd_system_call(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  uint16_t address = 0x0000u;
  args = string_skip_whitespace(args);
  if (!args) {
    mcode_errno_set(EArgument);
    return true;
  }
  args = string_next_number(args, &address);
  if (string_skip_whitespace(args)) {
    mcode_errno_set(EArgument);
    return true;
  }

#ifdef MCODE_COMMAND_MODES
  /* Make bootloader address available in non-root mode */
  if ((CmdModeRoot != cmd_engine_get_mode()) &&
      (CmdModeUser != cmd_engine_get_mode() || 0x7c00u != address)) {
    mcode_errno_set(EAccessDenied);
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

  const mcode_tick program = (mcode_tick)address;
#ifdef MCODE_WDT
  wdt_stop();
#endif /* MCODE_WDT */
  program();
#ifdef MCODE_WDT
  wdt_start();
#endif /* MCODE_WDT */
  return true;
}

#ifdef MCODE_BOOTLOADER
bool cmd_system_bootloader(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
#ifdef MCODE_COMMAND_MODES
  if (CmdModeRoot != cmd_engine_get_mode()) {
    mcode_errno_set(EAccessDenied);
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

#ifdef MCODE_WDT
  wdt_stop();
#endif /* MCODE_WDT */
  bootloader();
#ifdef MCODE_WDT
  wdt_start();
#endif /* MCODE_WDT */
  return true;
}
#endif /* MCODE_BOOTLOADER */
