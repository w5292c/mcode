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
#include "strings.h"
#include "hw-sound.h"

#include <avr/pgmspace.h>

static void cmd_engine_play_note(const char *args, bool *startCmd);

void cmd_engine_led_help(void)
{
  mprintstrln(PSTR("> sound <note> <length> - Play <note>, <length> msecs"));
}

bool cmd_engine_led_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("sound "), 6)) {
    cmd_engine_play_note(command + 6, startCmd);
    return true;
  }

  return false;
}

void cmd_engine_play_note(const char *args, bool *startCmd)
{
  uint16_t note = -1;
  uint16_t length = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &note);
  args = string_skip_whitespace(args);
  string_next_number(args, &length);

  if (note > 0xffu || !length) {
    merror(MStringWrongArgument);
    return;
  }

  sound_play_note((uint8_t)note, length);
}
