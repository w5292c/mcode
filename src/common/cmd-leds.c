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
#include "hw-leds.h"
#include "hw-uart.h"
#include "mglobal.h"
#include "strings.h"

static void cmd_engine_set_led(const char *args);

void cmd_engine_led_help(void)
{
  hw_uart_write_string_P(PSTR("> led <IND> <1/0> - Turn ON/OFF the LEDs\r\n"));
}

bool cmd_engine_led_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("led "), 4)) {
    cmd_engine_set_led(command + 4);
    return true;
  }

  return false;
}

void cmd_engine_set_led(const char *args)
{
  int index = -1;
  int value = -1;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &index);
  args = string_skip_whitespace(args);
  string_next_number(args, &value);

  if (index < 0 || index > 2 || value < 0 || value > 1) {
    merror(MStringWrongArgument);
    return;
  }

  mcode_hw_leds_set(index, value);
}
