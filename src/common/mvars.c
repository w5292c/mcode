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
static uint32_t TheIntBuffers[PROG_INTVARS_COUNT] = {0};
static const char *TheLabelVars[MCODE_LABELS_COUNT] = {NULL};
static char TheStringBuffers[PROG_STRVARS_COUNT][PROG_STRVAR_LENGTH] = {{0}};

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

const char *mvar_label(int index)
{
  if (index < MCODE_LABELS_COUNT) {
    return TheLabelVars[index];
  } else {
    return NULL;
  }
}

void mvar_label_set(int index, const char *label)
{
  if (index < MCODE_LABELS_COUNT) {
    TheLabelVars[index] = label;
  }
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

  type = var_parse_name(var, length, &idx, &cnt);
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
  } else if (VarTypeLabel == type) {
    const uint64_t value = (unsigned long)mvar_label(idx);
    mprintstr("0x");
    mprint_uint64(value, true);
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

typedef struct _VarNameMap {
  char letter;
  MVarType type;
} VarNameMap;

static const VarNameMap TheVarTypes[] = {
  { 'i', VarTypeInt },
  { 'n', VarTypeNvm },
  { 's', VarTypeString },
  { 'l', VarTypeLabel },
  { '\0', VarTypeNone }, /*< Map end marker */
};

MVarType var_parse_type(char ch)
{
  const VarNameMap *item = TheVarTypes;
  while (item->type != VarTypeNone) {
    if (ch == item->letter) {
      return item->type;
    }

    ++item;
  }

  return VarTypeNone;
}

/*
 * Parse the variable name
 * @param[in] name The variable name to check
 * @param[in] length The length of the variable name to check
 * @param[out] index The requested variable index
 * @param[out] count The requested variable count
 * @note The variable name follows the following format: <type><index>':'<count>,
 *       (this can be updated later), here:
 *       * <type> The variable type/variety, can be: 's', 'i', 'n', and 'l';
 *       * <index> The variable index: '0'..'9' (0 to 9) and 'a'..'z' (10 to 35);
 *       * <count> The variable length multiplier: '0'..'9' and 'a'..'z';
 * @note The format is not final, can be updated soon
 */
MVarType var_parse_name(const char *name, size_t length, size_t *index, size_t *count)
{
  char ch;
  bool skip;
  MVarType type;
  size_t idx = 0;
  size_t cnt = 1;
  if (!name || length < 1 || length > 4) {
    return VarTypeNone;
  }

  /* Extract the type of the variable */
  --length;
  type = var_parse_type(*name++);
  if (VarTypeNone == type) {
    return VarTypeNone;
  }

  /* Parse the 'index' field */
  if (length) {
    ch = *name++;
    --length;
    skip = false;
    if (ch >= '0' && ch <= '9') {
      idx = ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
      idx = ch - 'a' + 10;
    } else if (ch == ':') {
      /* ':' means that the index part is default ('0'), just move on to parsing next part */
      skip = true;
    } else {
      /* Invalid character detected, not a variable name */
      return VarTypeNone;
    }
  }

  /* Parse separator */
  if (!skip && length) {
    ch = *name++;
    --length;
    if (ch != ':') {
      /* Invalid character detected, not a variable name */
      return VarTypeNone;
    }
  }

  /* Parse the variable count */
  if (length) {
    ch = *name++;
    --length;
    if (ch >= '0' && ch <= '9') {
      cnt = ch - '0';
    } else if (ch >= 'a' && ch <= 'z') {
      cnt = ch - 'a' + 10;
    } else {
      return VarTypeNone;
    }
  }
  /* The following check must never be true, according to the previous length checks */
  /*
  if (count) {
    return VarTypeNone;
  }
  */

  /* Everything is parsed, report what is requested */
  if (index) {
    *index = idx;
  }
  if (count) {
    *count = cnt;
  }

  return type;
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
