/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Alexander Chumakov
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

#include "gsm-engine.h"

#include "allowed-phones.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

typedef void (*GsmEngineHandler)(const char *args, size_t length);

typedef enum {
  EGsmEngineCmdIdNone = 0,
  EGsmEngineCmdIdStatus,
  EGsmEngineCmdIdOn,
  EGsmEngineCmdIdOff,
} TGsmEngineCmdId;

typedef struct {
  const char *token;
  TGsmEngineCmdId id;
  GsmEngineHandler handler;
} TGsmEngineCmd;

static bool TheStatusReportEnabled = false;
static const char *const ThePhones[] = ALLOWED_PHONES;
static const TheCommands[] = {
  { EGsmEngineCmdIdStatus, "st" },
  { EGsmEngineCmdIdOn, "on" },
  { EGsmEngineCmdIdOff, "off" },
};

static void gsm_engine_send_status(void);
static bool gsm_engine_check_source(const char *source);
static void gsm_engine_cmd_on(const char *args, size_t length);
static void gsm_engine_cmd_off(const char *args, size_t length);
static void gsm_engine_cmd_status(const char *args, size_t length);
static void gsm_engine_process_line(const char *line, size_t length);

void gsm_handle_new_sms(const char *source, const char *body)
{
  TheStatusReportEnabled = false;

  /* Check if commands from the source phone number are allowed to process */
  if (!gsm_engine_check_source(source)) {
    return;
  }

  char ch;
  const char *ptr = body;
  while (true) {
    ch = *ptr++;

    /* Check for new-line/end-of-line markers */
    if (!ch || ch == '\r' || ch == '\n') {
      const size_t length = ptr - body - 1;
      if (length) {
        gsm_engine_process_line(body, length);
      }

      if (!ch) {
        break;
      }
      body = ptr;
    }
  }

  if (TheStatusReportEnabled) {
    gsm_engine_send_status();
  }
}

bool gsm_engine_check_source(const char *source)
{
  size_t i;
  bool allowed = false;
  const size_t count = sizeof (ThePhones)/sizeof (*ThePhones);
  for (i = 0; i < count; ++i) {
    if (!strcmp(ThePhones[i], source)) {
      allowed = true;
      break;
    }
  }

  return allowed;
}

void gsm_engine_process_line(const char *line, size_t length)
{
  const char *token = string_next_token(line, length);
  const size_t tokenLength = token - line;

  char buffer[256] = {0};
  strncpy(buffer, line, length);
  printf("Line, length: %d, value: \"%s\"\n", (int)length, buffer);
}

void gsm_engine_send_status(void)
{
}

void gsm_engine_cmd_on(const char *args, size_t length)
{
  //void switch_engine_turn_on(uint32_t ids, uint32_t seconds);
}

void gsm_engine_cmd_off(const char *args, size_t length)
{
  //void switch_engine_turn_off(uint32_t ids);
}

void gsm_engine_cmd_status(const char *args, size_t length)
{
  TheStatusReportEnabled = true;
}
