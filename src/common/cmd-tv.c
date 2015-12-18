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
#include "hw-uart.h"
#include "mglobal.h"
#include "persistent-store.h"

static bool cmd_engine_set_value(const char *args, bool *startCmd);

void cmd_engine_tv_help(void)
{
  hw_uart_write_string_P(PSTR("> value - Show persistent value\r\n"));
  hw_uart_write_string_P(PSTR("> value-set <number> - Update persistent value to <number>\r\n"));
}

bool cmd_engine_tv_command(const char *args, bool *startCmd)
{
  if (!strcmp_P(args, PSTR("value"))) {
    hw_uart_write_string_P(PSTR("Persistent value: "));
    hw_uart_write_uintd(persist_store_get_value(), 0);
    hw_uart_write_string_P(PSTR("\r\n"));
    return true;
  } else if (!strncmp_P(args, PSTR("value-set "), 10)) {
    return cmd_engine_set_value(args + 10, startCmd);
  }

  return false;
}

bool cmd_engine_set_value(const char *args, bool *startCmd)
{
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    hw_uart_write_string_P(PSTR("Error: wrong argument\r\n"));
    return true;
  }
  /* Get the 'number' argument */
  int number = -1;
  args = string_next_number(args, &number);
  if (!args || *args) {
    /* Wrong 'number' argument */
    hw_uart_write_string_P(PSTR("Error: wrong argument\r\n"));
    return true;
  }

  persist_store_set_value(number);
  return true;
}
