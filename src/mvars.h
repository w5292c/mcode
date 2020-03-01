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

typedef enum _MVarType {
  VarTypeNone,
  VarTypeInt,
  VarTypeNvm,
  VarTypeString,
} MVarType;

uint32_t mvar_int_get(int index);
void mvar_int_set(int index, uint32_t value);

uint16_t mvar_nvm_get(int index);
void mvar_nvm_set(int index, uint16_t value);

char *mvar_str(int index, int count, size_t *length);

void mvar_print(const char *var);

MVarType next_var(const char **str, size_t *length,
                  const char **token, uint32_t *value,
                  size_t *index, size_t *count);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_VARS_H */
