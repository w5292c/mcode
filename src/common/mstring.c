/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2020 Alexander Chumakov
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

#include "mstring.h"

#include "mvars.h"
#include "utils.h"
#include "mglobal.h"
#include "hw-uart.h"

#include <string.h>

static char *TheStringPutchPointer = NULL;
static const char *TheStringPutchPointerEnd = NULL;
static ostream_handler TheOutputHandler = NULL;

static size_t TheOutputStackCount = 0;
static ostream_handler TheOutputStack[MCODE_OUTPUT_STACK_COUNT] = {NULL};

void mputch(char ch)
{
  /* Check if we have a custom output stream handler */
  if (TheOutputHandler) {
    /* If we have a custom output stream handler, use it */
    (*TheOutputHandler)(ch);
    return;
  }

  /* If the output handler is not set, check the output handlers stack */
  if (TheOutputStackCount) {
    ostream_handler handler = TheOutputStack[TheOutputStackCount - 1];
    (*handler)(ch);
    return;
  }

  /* Default output stream handler */
  uart_write_char(ch);
}

void mputch_str(char ch)
{
  if (TheStringPutchPointer && TheStringPutchPointer < TheStringPutchPointerEnd)
  {
    *TheStringPutchPointer++ = ch;
  }
}

void mputch_str_config(char *buffer, size_t length)
{
  TheStringPutchPointer = buffer;
  TheStringPutchPointerEnd = buffer + length;
  memset(buffer, 0, length);
}

void io_set_ostream_handler(ostream_handler handler)
{
  TheOutputHandler = handler;
}

void io_ostream_handler_push(ostream_handler handler)
{
  if (TheOutputStackCount < MCODE_OUTPUT_STACK_COUNT - 1) {
    TheOutputStack[TheOutputStackCount++] = handler;
  }
}

void io_ostream_handler_pop(void)
{
  if (TheOutputStackCount) {
    --TheOutputStackCount;
  }
}

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
  mprintstr(mstring(id));
}

void mprintln(uint8_t id)
{
  mprint(id);
  mprint(MStringNewLine);
}

void mprintstrln(const char *string)
{
  mprintstr(string);
  mprint(MStringNewLine);
}

void mprintstr(const char *str)
{
  mprintbytes(str, -1);
}

void mprintbytes(const char *string, size_t length)
{
  if (string) {
    uint8_t ch;
    size_t bytes = 0;
    while (0 != (ch = pgm_read_byte(string++)) && (length == -1 || bytes++ < length)) {
      mputch(ch);
    }
  }
}

void mprintbytesln(const char *str, size_t length)
{
  mprintbytes(str, length);
  mprint(MStringNewLine);
}

void mprint_uintd(uint32_t value, uint8_t minDigits)
{
  if (!minDigits) {
    minDigits = 1;
  }
  uint8_t temp;
  for (temp = 10; temp < minDigits; ++temp) {
    mputch('0');
  }
  uint8_t digits = 10;
  bool keepZeroes = false;
  uint32_t factor = 1000000000U;
  while (factor) {
    temp = value/factor;
    if (temp || keepZeroes || digits <= minDigits) {
      mputch((char)temp + '0');
    }
    if (temp) {
      keepZeroes = true;
      value -= factor*temp;
    }

    factor /= 10;
    --digits;
  }
}

void mprint_uint64(uint64_t value, bool skipZeros)
{
  const uint32_t upper = (uint32_t)(value>>32);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    mprint_uint32(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  mprint_uint32((uint32_t)value, skipZeros);
}

void mprint_uint32(uint32_t value, bool skipZeros)
{
  const uint16_t upper = (uint16_t)(value>>16);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    mprint_uint16(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  mprint_uint16((uint16_t)value, skipZeros);
}

void mprint_uint16(uint16_t value, bool skipZeros)
{
  uint8_t i;
  char buffer[5];
  buffer[0] = nibble_to_char(0x0FU & (value >> 12));
  buffer[1] = nibble_to_char(0x0FU & (value >>  8));
  buffer[2] = nibble_to_char(0x0FU & (value >>  4));
  buffer[3] = nibble_to_char(0x0FU & value);
  buffer[4] = 0;
  if (skipZeros) {
    for (i = 0; i < 3; ++i) {
      if ('0' == *buffer) {
        memmove(buffer, buffer + 1, 4);
      }
    }
  }
  mprintstr_R(buffer);
}

void mprint_uint8(uint8_t value, bool skipZeros)
{
  char buffer[3];
  buffer[0] = nibble_to_char(0x0FU & (value >>  4));
  buffer[1] = nibble_to_char(0x0FU & value);
  buffer[2] = 0;

  if (skipZeros && '0' == *buffer) {
    memmove(buffer, buffer + 1, 2);
  }

  mprintstr_R(buffer);
}

void mprintstr_R(const char *string)
{
  if (string) {
    uint8_t ch;
    while (0 != (ch = (*string++))) {
      mputch(ch);
    }
  }
}

void mprintbytes_R(const char *string, size_t length)
{
  if (string) {
    uint8_t ch;
    size_t bytes = 0;
    while (0 != (ch = *string++) && (length == -1 || bytes++ < length)) {
      mputch(ch);
    }
  }
}

void mprintexpr(const char *str, size_t length)
{
  char ch;
  MVarType type;
  bool escape = false;

  /* Check the inputs */
  if (!str || !length) {
    /* Nothing to print */
    return;
  }
  if (-1 ==length) {
    length = strlen(str);
  }

  while (length) {
    /* Get another character */
    ch =*str++;
    --length;
    if (!ch) {
      break;
    }

    /* Fake loop, make it possible to use 'break' to skip this block, handling variables */
    while (!escape && '$' == ch) {
      size_t i;
      const char *ptr;

      if (!length) {
        /* last character in a string is '$', just print it */
        break;
      }
      /* Begin of a variable detected */
      ptr = str;
      ch = *ptr++;
      if ('$' == ch) {
        /* '$$' detected, print only one of them */
        str = ptr;
        --length;
        break;
      }
      /* Check if the item after '$' may look like a variable name */
      if (!char_is_letter(ch) && '_' != ch) {
        /* This does not look like a variable, just print '$' as usual character */
        ch = '$';
        break;
      }
      /* Detect 1st char that may not appear in a variable */
      for (i = 0; i < length; ++i) {
        ch = *ptr;
        if (!char_is_letter(ch) && ch != ':' && ch != '_' && !char_is_numeric(ch)) {
          break;
        }
        ++ptr;
      }
      /* Now, check if we detected the variable name */
      type = var_parse_name(str, ptr - str, NULL, NULL);
      if (VarTypeNone != type) {
        mvar_print(str, ptr - str);
        /* update the pointers to the next character */
        length -= ptr - str;
        str = ptr + 1;
      } else {
        /* This is not a variable, just print '$' as a usual characterr */
        ch = '$';
        break;
      }
      break;
    }
    if (!escape && '\\' == ch) {
      escape = true;
      continue;
    }
    if (escape) {
      escape = false;
      switch (ch) {
      case '0':
        ch = '\0';
        break;
      case 'a':
        ch = '\a';
        break;
      case 'b':
        ch = '\b';
        break;
      case 'e':
        ch = '\e';
        break;
      case 'f':
        ch = '\f';
        break;
      case 'n':
        ch = '\n';
        break;
      case 'r':
        ch = '\r';
        break;
      case 't':
        ch = '\t';
        break;
      case 'v':
        ch = '\v';
        break;
      default:
        break;
      }
    }
    mputch(ch);
  };
}

void mprint_dump_buffer(uint8_t length, const void *data, bool showAddress)
{
  if (!data) {
    return;
  }

  uint8_t i;
  bool newLineReported = true;
  for (i = 0; i < length; ++i) {
    /* Handle the new line */
    if (newLineReported) {
      if (showAddress) {
        mprint_uint32(i, false);
        mprintstr(PSTR("  "));
      }
      newLineReported = false;
    }

    /* Write hex data */
    mprint_uint8(*(*(const uint8_t **)&data)++, false);
    mputch(' ');

    if ((i & 0x0fu) == 0x0fu) {
      mprint(MStringNewLine);
      newLineReported = true;
    }
  }

  /* Move to the next line,
     if we have some text at the current line */
  if (!newLineReported) {
    mprint(MStringNewLine);
  }
}

void mprintstrhex16encoded(const char *str, size_t length)
{
  size_t i;

  if (-1 == length) {
    length = strlen(str);
  }

  for (i = 0; i < length; ++i) {
    const uint16_t ch = *str++;
    mprint_uint16(ch, false);
  }
}

void mprinthexencodedstr16(const char *str, size_t length)
{
  int i;

  if (-1 == length) {
    length = strlen(str);
  }
  const int n = length/4;

  for (i = 0; i < n; ++i, str += 4) {
    const uint16_t ch = glob_str_to_uint16(str);
    if (ch < 128) {
      mputch((char)ch);
    }
  }
}

void mprinthexencodeddata8(const void *data, size_t length)
{
  const uint8_t *ptr = data;
  while (length--) {
    mprint_uint8(*ptr++, false);
  }
}

const char *mstring(uint8_t id)
{
  switch (id) {
  case MStringNull:
    return PSTR("");
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
  case MStringWrongMode:
    return PSTR("only root can do this");
  case MStringErrorLimit:
    return PSTR("limit reached");
  case MStringEnterPass:
    return PSTR("Enter password: ");
  default:
    return PSTR("##undefined##");
  }
}
