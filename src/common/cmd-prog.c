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
}

bool cmd_engine_prog_command(const char *command, bool *startCmd)
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

void cmd_engine_prog_set(const char *args)
{
}

void cmd_engine_prog_print(const char *args)
{
}
