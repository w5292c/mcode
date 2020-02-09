/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Alexander Chumakov
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

#include "sha256.h"
#include "hw-twi.h"
#include "mstring.h"

#include <string.h>
#include <stdbool.h>

/**
 * EEPROM Memory Map:
 * ===========================================
 * |  Start | Length | Description           |
 * |========|========|=======================|
 * | 0x0000 |   0x20 | Password hash         |
 * | 0x0020 |   0x40 | Value storage         |
 * | 0x0060 |   0x02 | Initial value storage |
 * | 0x0062 |        | Free memory           |
 * ===========================================
 */

#ifdef __AVR__
#include <avr/eeprom.h>
static uint16_t TheDummyWord EEMEM __attribute__((used)) = 0xffffu;
#endif /* __AVR__ */

#define CYCLIC_STORAGE_ITEMS_COUNT (32)

void persist_store_load(uint8_t id, void *data, uint8_t length)
{
  if (PersistStoreIdHash != id || length != MD_LENGTH_SHA256) {
    return;
  }

  uint8_t buffer[2];
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  if (!twi_send_sync(0xaeu, 2, buffer)) {
    merror(MStringInternalError);
    return;
  }

  if (!twi_recv_sync(0xaeu, length, data)) {
    merror(MStringInternalError);
    return;
  }
}

void persist_store_save(uint8_t id, const void *data, uint8_t length)
{
  if (PersistStoreIdHash != id || length != MD_LENGTH_SHA256) {
    return;
  }

  uint8_t buffer[MD_LENGTH_SHA256 + 2];
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  memcpy(buffer + 2, data, length);

  if (!twi_send_sync(0xaeu, length + 2, buffer)) {
    merror(MStringInternalError);
    return;
  }
}

uint16_t persist_store_get_value(void)
{
  union {
    uint8_t buffer[2];
    uint16_t value;
  } u;
  uint8_t i;
  for (i = 31; i >= 0; --i) {
    /* Send address */
    u.buffer[0] = 0x00u;
    u.buffer[1] = 0x20u + (i<<1);
    if (!twi_send_sync(0xaeu, 2, u.buffer)) {
      return 0;
    }
    /* Read the value at index 'i' */
    if (!twi_recv_sync(0xaeu, 2, u.buffer)) {
      return 0;
    }
    if (u.value != 0xffffu) {
      break;
    }
  }
  return (u.value != 0xffffu) ? u.value : 0x0000u;
}

void persist_store_set_value(uint16_t value)
{
  if (persist_store_get_value() == value) {
    /* Already up-to-date value */
    return;
  }

  uint8_t i;
  union {
    uint8_t buffer[4];
    struct {
      uint16_t value;
      uint16_t value2;
    };
  } u;
  bool written = false;
  for (i = 0; i < CYCLIC_STORAGE_ITEMS_COUNT; ++i) {
    /* Send address */
    u.buffer[0] = 0x00u;
    u.buffer[1] = 0x20u + (i<<1);
    if (!twi_send_sync(0xaeu, 2, u.buffer)) {
      return;
    }
    /* Read the value at index 'i' */
    if (!twi_recv_sync(0xaeu, 2, u.buffer)) {
      return;
    }
    if (u.value == 0xffffu) {
      u.buffer[0] = 0x00u;
      u.buffer[1] = 0x20u + (i<<1);
      u.value2 = value;
      if (!twi_send_sync(0xaeu, 4, u.buffer)) {
        return;
      }
      written = true;
      break;
    }
  }

  /* No free space left, clean the buffer */
  if (!written) {
    for (i = 0; i < CYCLIC_STORAGE_ITEMS_COUNT; ++i) {
      u.buffer[0] = 0x00u;
      u.buffer[1] = 0x20u + (i<<1);
      u.value2 = i ? 0xffffu : value;
      if (!twi_send_sync(0xaeu, 4, u.buffer)) {
        return;
      }
    }
  }
}

uint16_t persist_store_get_initial_value(void)
{
  union {
    uint8_t buffer[2];
    uint16_t value;
  } u;
  u.buffer[0] = 0x00u;
  u.buffer[1] = 0x60u;
  if (!twi_send_sync(0xaeu, 2, u.buffer)) {
    merror(MStringInternalError);
    return 0;
  }

  if (!twi_recv_sync(0xaeu, 2, u.buffer)) {
    merror(MStringInternalError);
    return 0;
  }

  return u.value;
}

void persist_store_set_initial_value(uint16_t value)
{
  uint8_t buffer[4];
  buffer[0] = 0x00u;
  buffer[1] = 0x60u;
  memcpy(buffer + 2, &value, 2);

  if (!twi_send_sync(0xaeu, 4, buffer)) {
    merror(MStringInternalError);
    return;
  }
}
