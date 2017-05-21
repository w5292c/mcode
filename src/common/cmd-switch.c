/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Alexander Chumakov
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
#include "mglobal.h"
#include "mstring.h"
#include "switch-engine.h"

#include <string.h>

static void cmd_engine_switch_turn_on(const char *args);
static void cmd_engine_switch_turn_off(const char *args);

void cmd_engine_switch_help(void)
{
  mprintstrln(PSTR("> switch-off <IND> - Turn OFF the switch #IND"));
  mprintstrln(PSTR("> switch-on <IND> <TIMEOUT> - Turn the switch #IND ON for TIMEOUT seconds"));
}

bool cmd_engine_led_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("switch-on "), 10)) {
    cmd_engine_switch_turn_on(command + 10);
    return true;
  } else if (!strncmp_P(command, PSTR("switch-off "), 11)) {
    cmd_engine_switch_turn_off(command + 11);
    return true;
  }

  return false;
}

void cmd_engine_switch_turn_on(const char *args)
{
  uint16_t index = -1;
  uint16_t seconds = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &index);
  args = string_skip_whitespace(args);
  string_next_number(args, &seconds);

  if (index > 1u) {
    merror(MStringWrongArgument);
    return;
  }

  switch_engine_turn_on(index, seconds);
}

void cmd_engine_switch_turn_off(const char *args)
{
  uint16_t index = -1;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &index);

  if (index > 1u) {
    merror(MStringWrongArgument);
    return;
  }

  switch_engine_turn_off(index);
}
