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

#include "sha.h"
#include "hw-twi.h"
#include "mstring.h"
#include "scheduler.h"

#include <string.h>
#include <stdbool.h>

/**
 * DS1307 Memory Map:
 * ===========================================
 * |  Start | Length | Description           |
 * |========|========|=======================|
 * | 0x0000 |   0x08 | Date/time data        |
 * | 0x0008 |   0x02 | Value storage         |
 * | 0x000a |   0x02 | Initial value storage |
 * | 0x000c |   0x04 | Free memory           |
 * | 0x0010 |   0x20 | Password hash         |
 * | 0x0030 |   0x10 | Free memory           |
 * ===========================================
 */

void persist_store_load(uint8_t id, uint8_t *data, uint8_t length)
{
  if (PersistStoreIdHash != id || length != SHA256_DIGEST_LENGTH) {
    merror(MStringWrongArgument);
    return;
  }

  const uint8_t buffer = 0x10u;
  if (!twi_send_sync(0xd0u, 1, &buffer)) {
    merror(MStringInternalError);
    return;
  }
  if (!twi_recv_sync(0xd0u, length, data)) {
    merror(MStringInternalError);
    return;
  }
}

void persist_store_save(uint8_t id, const uint8_t *data, uint8_t length)
{
  if (PersistStoreIdHash != id || length != SHA256_DIGEST_LENGTH) {
    merror(MStringWrongArgument);
    return;
  }

  uint8_t buffer[SHA256_DIGEST_LENGTH + 1];
  buffer[0] = 0x10u;
  memcpy(buffer + 1, data, length);
  if (!twi_send_sync(0xd0u, SHA256_DIGEST_LENGTH + 1, buffer)) {
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
  u.buffer[0] = 0x08u;
  if (!twi_send_sync(0xd0u, 1, u.buffer)) {
    merror(MStringInternalError);
    return 0;
  }

  if (!twi_recv_sync(0xd0u, 2, u.buffer)) {
    merror(MStringInternalError);
    return 0;
  }

  return u.value;
}

void persist_store_set_value(uint16_t value)
{
  uint8_t buffer[3];
  buffer[0] = 0x08u;
  memcpy(buffer + 1, &value, 2);
  if (!twi_send_sync(0xd0u, 3, buffer)) {
    merror(MStringInternalError);
    return;
  }
}

uint16_t persist_store_get_initial_value(void)
{
  union {
    uint8_t buffer[2];
    uint16_t value;
  } u;
  u.buffer[0] = 0x0au;
  if (!twi_send_sync(0xd0u, 1, u.buffer)) {
    merror(MStringInternalError);
    return 0;
  }

  if (!twi_recv_sync(0xd0u, 2, u.buffer)) {
    merror(MStringInternalError);
    return 0;
  }

  return u.value;
}

void persist_store_set_initial_value(uint16_t value)
{
  uint8_t buffer[3];
  buffer[0] = 0x0au;
  memcpy(buffer + 1, &value, 2);
  if (!twi_send_sync(0xd0u, 3, buffer)) {
    merror(MStringInternalError);
    return;
  }
}
