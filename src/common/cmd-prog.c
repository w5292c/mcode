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

#include "cmd-engine.h"

#include "utils.h"
#include "hw-nvm.h"
#include "mglobal.h"
#include "mstring.h"

#include <string.h>

#define PROG_INTVARS_COUNT (16)
#define PROG_STRVARS_COUNT (16)
#define PROG_STRVAR_LENGTH (128)

static uint32_t TheIntBuffers[PROG_INTVARS_COUNT];
static char TheStringBuffers[PROG_STRVARS_COUNT][PROG_STRVAR_LENGTH];

static void cmd_engine_prog_set(const char *args);
static void cmd_engine_prog_print(const char *args);
static void cmd_engine_prog_execute(const char *args);

void cmd_engine_prog_help(void)
{
  mprintstrln(PSTR("> prog [options] Access programming interface"));
  mprintstrln(PSTR("> prog print <var> - Print value of <var>"));
  mprintstrln(PSTR("> prog set <var> <value> - Set <var> to <value>"));
  mprintstrln(PSTR(">          <var>: s0 - string, i0 - int, n0 - NVM int"));
}

bool cmd_engine_prog_exec(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("prog "), 5)) {
    cmd_engine_prog_execute(command + 5);
    return true;
  }

  return false;
}

void cmd_engine_prog_execute(const char *args)
{
  if (!strncmp_P(args, PSTR("print "), 6)) {
    cmd_engine_prog_print(args + 6);
  } else if (!strncmp_P(args, PSTR("set "), 4)) {
    cmd_engine_prog_set(args + 4);
  }
}

void cmd_engine_prog_print(const char *args)
{
  uint16_t index;
  const char var_name = *args++;
  args = string_next_decimal_number(args, &index);
  if (args ||
    ('s' == var_name && index >= PROG_STRVARS_COUNT) ||
    ('i' == var_name && index >= PROG_INTVARS_COUNT) ||
    ('n' == var_name && index >= 10)) {
    merror(MStringWrongArgument);
    return;
  }

  if ('s' == var_name) {
    mputch('\'');
    mprintstr_R(TheStringBuffers[index]);
    mputch('\'');
  } else if ('i' == var_name) {
    mprint_uintd(TheIntBuffers[index], 1);
  } else if ('n' == var_name) {
    mprint_uintd(nvm_read(index), 1);
  } else {
    merror(MStringWrongArgument);
    return;
  }
  mprint(MStringNewLine);
}

void cmd_engine_prog_set(const char *args)
{
  uint16_t index;
  uint16_t value;
  const char var_name = *args++;
  args = string_next_decimal_number(args, &index);
  if (!args ||
    ('s' == var_name && index >= PROG_STRVARS_COUNT) ||
    ('i' == var_name && index >= PROG_INTVARS_COUNT) ||
    ('n' == var_name && index >= 10)) {
    merror(MStringWrongArgument);
    return;
  }

  if ('i' == var_name || 'n' == var_name) {
    args = string_skip_whitespace(args);
    if (!args) {
      merror(MStringWrongArgument);
      return;
    }
    args = string_next_number(args, &value);
    if (args) {
      merror(MStringWrongArgument);
      return;
    }
    if ('i' == var_name) {
      TheIntBuffers[index] = value;
    } else {
      nvm_write(index, value);
    }
  } else if ('s' == var_name && *args++) {
    const size_t length = strlen(args);
    if (length < PROG_STRVAR_LENGTH) {
      memset(TheStringBuffers[index], 0, PROG_STRVAR_LENGTH);
      memcpy(TheStringBuffers[index], args, length);
    } else {
      merror(MStringWrongArgument);
    }
  } else {
    merror(MStringWrongArgument);
  }
}
