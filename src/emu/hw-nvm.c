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

#include "hw-nvm.h"

#include "persistent-store.h"

#include <stdio.h>

uint16_t nvm_read(uint_least8_t index)
{
  uint16_t values[MCODE_NVM_MAX_INDEX] = {0};
  if (index < MCODE_NVM_MAX_INDEX) {
    persist_store_load(PersistStoreIdNvm, values, sizeof (values));
    return values[index];
  }
  fprintf(stderr, "Error: no NVM element at index: %d\n", index);
  return 0;
}

void nvm_write(uint_least8_t index, uint16_t value)
{
  uint16_t values[MCODE_NVM_MAX_INDEX] = {0};
  if (index < MCODE_NVM_MAX_INDEX) {
    persist_store_load(PersistStoreIdNvm, values, sizeof (values));
    values[index] = value;
    persist_store_save(PersistStoreIdNvm, values, sizeof (values));
  } else {
    fprintf(stderr, "Error: no NVM element at index: %d\n", index);
  }
}
