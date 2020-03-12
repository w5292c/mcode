/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Alexander Chumakov
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

#include "mglobal.h"
#include "mparser.h"
#include "mstring.h"

#include <stddef.h>

static bool cmd_help_command_handler(const TCmdData *data, const char *args,
                                     size_t args_len, bool *start_cmd);

static const char TheHelpMessage[] PROGMEM = ("Show help for all commands");

CMD_ENTRY(PSTR("help"), help, TheHelpMessage, &cmd_help_command_handler, NULL, 0);

bool cmd_help_command_handler(const TCmdData *data, const char *args,
                              size_t args_len, bool *start_cmd)
{
  const TCmdData *iter = &__start_command_section;
  const TCmdData *const end = &__stop_command_section;

  if (!args_len) {
    /* List all supported commands */
    bool first_cmd = true;
    mprintstr(PSTR("Commands: ["));
    for (; iter < end; ++ iter) {
      if (!first_cmd) {
        mprintstr(PSTR(", "));
      }
      const char *const base = pgm_read_ptr_near(&iter->base);
      mprintstr(base);
      first_cmd = false;
    }
    mprintstrln(PSTR("]"));
  } else {
    /* Show help for a specific command */
    for (; iter < end; ++ iter) {
      const char *const base = pgm_read_ptr_near(&iter->base);
      if (!mparser_strcmp_P(args, args_len, base)) {
        mprintstr(PSTR("> "));
        mprintbytes_R(args, args_len);
        mprintstr(PSTR(" - "));
        const char *const help = pgm_read_ptr_near(&iter->help);
        mprintstr(help);
        break;
      }
    }
    mprint(MStringNewLine);
  }

  return true;
}
