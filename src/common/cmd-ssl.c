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

#include "cmd-engine.h"

#include "sha.h"
#include "hw-uart.h"
#include "mglobal.h"

#include <string.h>

static void cmd_engine_sha256(const char *aParams);

void cmd_engine_ssl_help(void)
{
  hw_uart_write_string_P(PSTR("> sha256 <DATA> - calculate SHA256 hash\r\n"));
}

bool cmd_engine_ssl_command(const char *args, bool *startCmd)
{
  if (!strncmp_P(args, PSTR("sha256 "), 7)) {
    cmd_engine_sha256(&args[7]);
    return true;
  }

  return false;
}

void cmd_engine_sha256(const char *aParams)
{
  const uint16_t n = strlen(aParams);
  hw_uart_write_string_P(PSTR("Calculating sha256 hash, string: '"));
  hw_uart_write_string(aParams);
  hw_uart_write_string_P(PSTR("', length: 0x"));
  hw_uart_write_uint(n);
  hw_uart_write_string_P(PSTR("\r\n"));

  uint8_t byteResultHash[SHA256_DIGEST_LENGTH];
  SHA256((const unsigned char *)aParams, n, byteResultHash);

  uint8_t i;
  uint8_t *ptr = byteResultHash;
  for (i = 0; i < SHA256_DIGEST_LENGTH; i += 2, ptr += 2) {
    uint16_t data = ((*ptr) << 8) | (*(ptr + 1) << 0);
    hw_uart_write_uint16(data, false);
  }
  hw_uart_write_string_P(PSTR("\r\n"));
}
