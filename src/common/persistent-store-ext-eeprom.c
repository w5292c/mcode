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

static void persist_store_write_ready(bool success);
static void persist_store_read_ready(bool success, uint8_t length, const uint8_t *data);

static bool TheSuccess;
static uint8_t *TheBuffer;

void persist_store_load(uint8_t id, uint8_t *data, uint8_t length)
{
  if (PersistStoreIdHash != id || length != SHA256_DIGEST_LENGTH) {
    return;
  }

  uint8_t buffer[2];
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  TheSuccess = false;
  twi_send(0xaeu, 2, buffer, persist_store_write_ready);
  mcode_scheduler_start();

  if (!TheSuccess) {
    merror(MStringInternalError);
    return;
  }

  TheSuccess = false;
  TheBuffer = data;
  twi_recv(0xaeu, length, persist_store_read_ready);
  mcode_scheduler_start();

  if (!TheSuccess) {
    merror(MStringInternalError);
  }
}

void persist_store_save(uint8_t id, const uint8_t *data, uint8_t length)
{
  if (PersistStoreIdHash != id || length != SHA256_DIGEST_LENGTH) {
    return;
  }

  uint8_t buffer[SHA256_DIGEST_LENGTH + 2];
  buffer[0] = 0x00u;
  buffer[1] = 0x00u;
  memcpy(buffer + 2, data, length);

  TheSuccess = false;
  twi_send(0xaeu, length + 2, buffer, persist_store_write_ready);
  mcode_scheduler_start();

  if (!TheSuccess) {
    merror(MStringInternalError);
  }
}

void persist_store_write_ready(bool success)
{
  TheSuccess = success;
  mcode_scheduler_stop();
}

void persist_store_read_ready(bool success, uint8_t length, const uint8_t *data)
{
  TheSuccess = success;
  if (success) {
    memcpy(TheBuffer, data, length);
  }
  mcode_scheduler_stop();
}

uint16_t persist_store_get_initial_value(void)
{
  union {
    uint8_t buffer[2];
    uint16_t value;
  } u;
  u.buffer[0] = 0x00u;
  u.buffer[1] = 0x60u;
  TheSuccess = false;
  twi_send(0xaeu, 2, u.buffer, persist_store_write_ready);
  mcode_scheduler_start();

  if (!TheSuccess) {
    merror(MStringInternalError);
    return 0;
  }

  TheSuccess = false;
  TheBuffer = u.buffer;
  twi_recv(0xaeu, 2, persist_store_read_ready);
  mcode_scheduler_start();

  if (!TheSuccess) {
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

  TheSuccess = false;
  twi_send(0xaeu, 4, buffer, persist_store_write_ready);
  mcode_scheduler_start();

  if (!TheSuccess) {
    merror(MStringInternalError);
  }
}
