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

#include "strings.h"

#include "mglobal.h"
#include "hw-uart.h"

static void mcode_out(const char *value);

void merror(uint8_t id)
{
  mprint(MStringError);
  mprintln(id);
}

void mwarning(uint8_t id)
{
  mprint(MStringWarning);
  mprintln(id);
}

void mprint(uint8_t id)
{
  mcode_out(mstring(id));
}

void mprintln(uint8_t id)
{
  mprint(id);
  mprint(MStringNewLine);
}

void mprintstrln(const char *string)
{
  mcode_out(string);
  mprint(MStringNewLine);
}

void mcode_out(const char *value)
{
  hw_uart_write_string_P(value);
}

const char *mstring(uint8_t id)
{
  switch (id) {
  case MStringNull:
    return NULL;
  case MStringNewLine:
    return PSTR("\r\n");
  case MStringError:
    return PSTR("Error: ");
  case MStringWarning:
    return PSTR("Warning: ");
  case MStringInternalError:
    return PSTR("internal error");
  case MStringWrongArgument:
    return PSTR("wrong argument(s)");
  default:
    return PSTR("##undefined##");
  }
}
