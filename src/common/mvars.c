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

#include "mvars.h"

#include "utils.h"
#include "hw-nvm.h"
#include "mstring.h"

#include <string.h>

static char *TheStringPutchPointer = NULL;
static const char *TheStringPutchPointerEnd = NULL;
static uint32_t TheIntBuffers[PROG_INTVARS_COUNT];
static char TheStringBuffers[PROG_STRVARS_COUNT][PROG_STRVAR_LENGTH];

uint32_t mvar_int_get(int index)
{
  if (index < PROG_INTVARS_COUNT) {
    return TheIntBuffers[index];
  } else {
    return 0;
  }
}

void mvar_int_set(int index, uint32_t value)
{
  if (index < PROG_INTVARS_COUNT) {
    TheIntBuffers[index] = value;
  }
}

uint16_t mvar_nvm_get(int index)
{
  return nvm_read(index);
}

void mvar_nvm_set(int index, uint16_t value)
{
  nvm_write(index, value);
}

char *mvar_str(int index, int count, size_t *length)
{
  if (index < PROG_STRVARS_COUNT) {
    if (length) {
      int last = index + count;
      if (last > PROG_STRVARS_COUNT) {
        last = PROG_STRVARS_COUNT;
      }
      *length = PROG_STRVAR_LENGTH*(last - index);
    }
    return TheStringBuffers[index];
  } else {
    return NULL;
  }
}

void mvar_print(const char *var, size_t length)
{
  size_t idx = 0;
  size_t cnt = 1;
  const char *token = NULL;
  uint32_t token_length = 0;
  MVarType type = VarTypeNone;

  if (!var) {
    return;
  }

  if (-1 == length) {
    /* Update the length, if it is not defined */
    length = strlen(var);
  }

  type = next_var(&var, &length, &token, &token_length, &idx, &cnt);
  if (VarTypeNone == type) {
    /* The variable name is not correct/not found */
    return;
  }

  if (VarTypeString == type) {
    size_t length = 0;
    const char *const str = mvar_str(idx, cnt, &length);
    if (str && length) {
      mprintbytes(str, length);
    }
  } else {
    uint32_t value = 0;
    if (VarTypeNvm == type) {
      value = mvar_nvm_get(idx);
      if (cnt > 1) {
        /* Combine 2nd 16-bit word in LE style */
        value |= (mvar_nvm_get(idx + 1) << 16);
      }
    } else {
      value = mvar_int_get(idx);
    }
    mprint_uintd(value, 1);
  }
}

MVarType next_var(const char **str, size_t *length,
                  const char **token, uint32_t *value, size_t *index, size_t *count)
{
  /*
   * Variable names:
   * - s; n; i; (at index '0' on default, count - 1);
   * - s1; n3; i6; (at defined index, count - 1);
   * - s2:2; i5:4; n1:2; (at provided index and with defined count);
   * Indeces: 0..9, a..z, a refers to 10, b - 11, etc...;
   * Possible extensions: s{1:2}, to be implemented later, if needed;
   */
  char ch;
  const char *ptr;
  const char *nxt;

  if (!str || !length || !token || !value || !index || !count) {
    return VarTypeNone;
  }

  ptr = *str;
  nxt = ptr;
  size_t len = *length;
  if (!ptr || !len) {
    return VarTypeNone;
  }

  char type = 0;
  int idx = 0;
  int cnt = 1;
  do {
    type = *ptr++;
    --len;
    if ('s' != type && 'i' != type && 'n' != type) {
      /* Wrong variable name */
      break;
    }
    nxt = ptr;
    if (!len) {
      /* No more variable spec */
      break;
    }
    ch = *ptr++;
    --len;
    if (ch >= '0' && ch <= '9') {
      nxt = ptr;
      idx = ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
      nxt = ptr;
      idx = ch - 'a' + 10;
    } else {
      /* Wrong character */
      break;
    }
    if (!len) {
      /* String is finished */
      break;
    }
    if (':' != (*ptr++)) {
      /* Variable spec finished */
      break;
    }
    nxt = ptr;
    ch = *ptr++;
    --len;
    if (ch >= '0' && ch <= '9') {
      nxt = ptr;
      cnt = ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
      nxt = ptr;
      cnt = ch - 'a' + 10;
    }
  } while (false);

  if ('s' != type && 'i' != type && 'n' != type) {
    /* No variable detected */
    return VarTypeNone;
  }

  /* Copy the variable name length */
  *value = nxt - *str;
  /* Copy the variable name pointer */
  *token = *str;
  *str = nxt;
  *index = idx;
  *count = cnt;
  *length -= *value;
  return (type == 's') ? VarTypeString :
         (type == 'i') ? VarTypeInt : VarTypeNvm;
}

void mvar_putch(char ch)
{
  if (TheStringPutchPointer && TheStringPutchPointer < TheStringPutchPointerEnd) {
    *TheStringPutchPointer++ = ch;
  }
}

void mvar_putch_config(int index, int count)
{
  size_t length = 0;
  TheStringPutchPointer = mvar_str(index, count, &length);
  if (TheStringPutchPointer) {
    /* Reserve 1 byte for end-of-string marker \0 */
    if (length) {
      TheStringPutchPointerEnd = TheStringPutchPointer + length - 1;
      memset(TheStringPutchPointer, 0, length);
    } else {
      TheStringPutchPointerEnd = TheStringPutchPointer;
    }
  } else {
    TheStringPutchPointerEnd = NULL;
  }
}
