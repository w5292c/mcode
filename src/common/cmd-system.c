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

#include "mtick.h"
#include "utils.h"
#include "hw-uart.h"
#include "mglobal.h"

static bool cmd_engine_sleep(const char *args, bool *startCmd);

void cmd_engine_system_help(void)
{
  hw_uart_write_string_P(PSTR("> sleep <msec> - Suspend execution for <msec> milli-seconds\r\n"));
}

bool cmd_engine_system_command(const char *args, bool *startCmd)
{
  if (!strncmp_P(args, PSTR("sleep"), 5)) {
    return cmd_engine_sleep(args + 5, startCmd);
  }

  return false;
}

bool cmd_engine_sleep(const char *args, bool *startCmd)
{
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(PSTR("Error: wrong argument\r\n"));
    return true;
  }
  /* Get the 'mticks' argument */
  int mticks = -1;
  args = string_next_number(args, &mticks);
  if (!args || *args || mticks < 0) {
    /* Wrong 'mticks' argument */
    hw_uart_write_string_P(PSTR("Error: wrong argument\r\n"));
    return true;
  }

  mtick_sleep(mticks);
  return true;
}
