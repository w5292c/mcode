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

#include "persistent-store.h"

#include "hw-nvm.h"

#include <string.h>

/* hash for the initial passwd: 'pass' */
static uint8_t TheHash[32] = {
  0xd7u, 0x4fu, 0xf0u, 0xeeu, 0x8du, 0xa3u, 0xb9u, 0x80u,
  0x6bu, 0x18u, 0xc8u, 0x77u, 0xdbu, 0xf2u, 0x9bu, 0xbdu,
  0xe5u, 0x0bu, 0x5bu, 0xd8u, 0xe4u, 0xdau, 0xd7u, 0xa3u,
  0xa7u, 0x25u, 0x00u, 0x0fu, 0xebu, 0x82u, 0xe8u, 0xf1u,
};

static uint16_t TheNvmValues[MCODE_NVM_MAX_INDEX] = {0};

void persist_store_load(uint8_t id, void *data, uint8_t length)
{
  const void *pointer = NULL;
  switch (id) {
  case PersistStoreIdHash:
    pointer = TheHash;
    break;
  case PersistStoreIdNvm:
    pointer = TheNvmValues;
    break;
  default:
    return;
  }

  memcpy(data, pointer, length);
}

void persist_store_save(uint8_t id, const void *data, uint8_t length)
{
  void *pointer = NULL;
  switch (id) {
  case PersistStoreIdHash:
    pointer = TheHash;
    break;
  case PersistStoreIdNvm:
    pointer = TheNvmValues;
    break;
  default:
    return;
  }

  memcpy(pointer, data, length);
}
