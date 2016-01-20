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
#include "hw-twi.h"
#include "mglobal.h"
#include "mstring.h"

static void cmd_engine_twi_read(const char *args);
static void cmd_engine_twi_write(const char *args);

void cmd_engine_twi_help(void)
{
  mprintstrln(PSTR("> twi-rd <addr> <length> - Read <length> bytes from TWI device at <addr>"));
  mprintstrln(PSTR("> twi-wr <addr> <hex-data> - Write <hex-data> to TWI device at <addr>"));
}

bool cmd_engine_twi_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("twi-rd "), 7)) {
    cmd_engine_twi_read(command + 7);
    return true;
  } else if (!strncmp_P(command, PSTR("twi-wr "), 7)) {
    cmd_engine_twi_write(command + 7);
    return true;
  }

  return false;
}

void cmd_engine_twi_read(const char *args)
{
  /* move to the arguments start */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    merror(MStringWrongArgument);
    return;
  }
  /* Parse the TWI address */
  uint16_t twi_addr = 0;
  args = string_next_number(args, &twi_addr);
  if (!args || !twi_addr) {
    merror(MStringWrongArgument);
    return;
  }
  /* Parse the TWI read request length */
  uint16_t twi_length = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &twi_length);
  if (args || !twi_length || twi_length > 32) {
    merror(MStringWrongArgument);
    return;
  }

  uint8_t buffer[32];
  if (!twi_recv_sync(twi_addr, twi_length, buffer)) {
    merror(MStringInternalError);
    return;
  }

  mprintstrln(PSTR("TWI read data:"));
  mprint_dump_buffer(twi_length, buffer, true);
}

void cmd_engine_twi_write(const char *args)
{
  /* Parse the TWI address */
  uint16_t twi_addr = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &twi_addr);
  if (!args || !twi_addr) {
    merror(MStringWrongArgument);
    return;
  }

  uint8_t buffer[32];
  uint8_t bufferFilled = 0;
  args = string_skip_whitespace(args);
  args = string_to_buffer(args, 32, buffer, &bufferFilled);
  if (args || !bufferFilled) {
    merror(MStringWrongArgument);
    return;
  }

  if (!twi_send_sync(twi_addr, bufferFilled, buffer)) {
    merror(MStringInternalError);
    return;
  }
}
