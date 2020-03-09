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

#ifndef MCODE_VARS_H
#define MCODE_VARS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MCODE_LABELS_COUNT (4)
#define PROG_INTVARS_COUNT (16)
#define PROG_STRVARS_COUNT (16)
#define PROG_STRVAR_LENGTH (128)

typedef enum _MVarType {
  VarTypeNone,
  VarTypeInt,
  VarTypeNvm,
  VarTypeString,
  VarTypeLabel,
} MVarType;

uint32_t mvar_int_get(int index);
void mvar_int_set(int index, uint32_t value);

uint16_t mvar_nvm_get(int index);
void mvar_nvm_set(int index, uint16_t value);

char *mvar_str(int index, int count, size_t *length);

const char *mvar_label(int index);
void mvar_label_set(int index, const char *label);

void mvar_print(const char *var, size_t length);

MVarType next_var(const char **str, size_t *length,
                  const char **token, uint32_t *value,
                  size_t *index, size_t *count);

/**
 * Parse the string passed in \c name and \c length if it is a variable name
 * @param[in] name The string to check if it represents
 * @param[in] length The \c name string length
 * @param[out] index The index of the variable, may be \c NULL
 * @param[out] count The count of the variable, may be \c NULL
 * @return The type of variable or \c VarTypeNone if not a variable name is passed in \c name
 * @note The out-parameters \c index and \c count only retured,
 *       if the input data represents a variable name
 */
MVarType var_parse_name(const char *name, size_t length, size_t *index, size_t *count);

/**
 * Write the character passed in \c ch to the previously configured string variable
 * @param[in] ch The character to be added to the string variable
 * @note If the currently configured variable does not have enough space for the new character,
 *       the request is ignored.
 */
void mvar_putch(char ch);

/**
 * Configure the \c mvar_putch requests
 * @param[in] index The start index of the output string variable for \c mvar_putch requests
 * @param[in] count The number of blocks for the output string variables for \c mvar_putch requests
 * @note The \c index and \c count are checked for correctness before applying, if we pass invalid
 *       values, they are rounded to the nearest correct values
 * @note This request also resets the variable buffer, feeling with with \0 character
 */
void mvar_putch_config(int index, int count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_VARS_H */
