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

#include "persistent-store.h"

#include "mstring.h"
#include "mcode-config.h"

#include <stdbool.h>
#include <avr/eeprom.h>

#ifndef MCODE_PERSIST_STORE_EXT_EEPROM
/* hash for the initial passwd: 'pass' */
static uint8_t TheHash[32] EEMEM = {
  0xd7u, 0x4fu, 0xf0u, 0xeeu, 0x8du, 0xa3u, 0xb9u, 0x80u,
  0x6bu, 0x18u, 0xc8u, 0x77u, 0xdbu, 0xf2u, 0x9bu, 0xbdu,
  0xe5u, 0x0bu, 0x5bu, 0xd8u, 0xe4u, 0xdau, 0xd7u, 0xa3u,
  0xa7u, 0x25u, 0x00u, 0x0fu, 0xebu, 0x82u, 0xe8u, 0xf1u,
};
#endif /* MCODE_PERSIST_STORE_EXT_EEPROM */

#define CYCLIC_STORAGE_LENGTH (32)
static uint16_t TheCyclicStorage[CYCLIC_STORAGE_LENGTH] EEMEM = {
  0x003cu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu,
  0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu,
  0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu,
  0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu, 0xffffu,
};

#ifndef MCODE_PERSIST_STORE_EXT_EEPROM
static uint16_t TheInitailValue EEMEM = 60;
#endif /* MCODE_PERSIST_STORE_EXT_EEPROM */

#ifndef MCODE_PERSIST_STORE_EXT_EEPROM
void persist_store_load(uint8_t id, uint8_t *data, uint8_t length)
{
  const uint8_t *pointer = NULL;
  switch (id) {
  case PersistStoreIdHash:
    pointer = TheHash;
    break;
  default:
    return;
  }

  eeprom_read_block(data, pointer, length);
}

void persist_store_save(uint8_t id, const uint8_t *data, uint8_t length)
{
  uint8_t *pointer = NULL;
  switch (id) {
  case PersistStoreIdHash:
    pointer = TheHash;
    break;
  default:
    return;
  }

  eeprom_write_block(data, pointer, length);
}
#endif /* MCODE_PERSIST_STORE_EXT_EEPROM */

uint16_t persist_store_get_value(void)
{
  uint8_t i;
  uint16_t result = 0;
  for (i = 31; i >= 0; --i) {
    const uint16_t word = eeprom_read_word(&TheCyclicStorage[i]);
    if (word != 0xffffu) {
      result = word;
      break;
    }
  }
  return result;
}

void persist_store_set_value(uint16_t value)
{
  if (persist_store_get_value() == value) {
    /* Already valid value */
    return;
  }

  uint8_t i;
  bool written = false;
  for (i = 0; i < CYCLIC_STORAGE_LENGTH; ++i) {
    const uint16_t word = eeprom_read_word(&TheCyclicStorage[i]);
    if (word == 0xffffu) {
      written = true;
      eeprom_write_word(&TheCyclicStorage[i], value);
      break;
    }
  }

  /* No free space left, clean the buffer */
  if (!written) {
    for (i = 0; i < CYCLIC_STORAGE_LENGTH; ++i) {
      eeprom_write_word(&TheCyclicStorage[i], (0 == i) ? value : 0xffffu);
    }
  }
}

#ifndef MCODE_PERSIST_STORE_EXT_EEPROM
uint16_t persist_store_get_initial_value(void)
{
  return eeprom_read_word(&TheInitailValue);
}

void persist_store_set_initial_value(uint16_t value)
{
  eeprom_write_word(&TheInitailValue, value);
}
#endif /* MCODE_PERSIST_STORE_EXT_EEPROM */

#if 0 /* Test code: begin */
void persist_store_test(void)
{
  uint8_t i;
  bool newLineReported = true;
  for (i = 0; i < CYCLIC_STORAGE_LENGTH; ++i) {
    /* Handle the new line */
    if (newLineReported) {
      mprint_uint32(i, false);
      mprintstr(PSTR("  "));
      newLineReported = false;
    }

    /* Write hex data */
    const uint16_t data = eeprom_read_word(&TheCyclicStorage[i]);
    mprint_uint16(data, false);
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
#endif /* Test code: end */
