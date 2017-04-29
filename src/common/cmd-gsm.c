/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Alexander Chumakov
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

#include "utils.h"
#include "mtick.h"
#include "hw-uart.h"
#include "mglobal.h"
#include "mstring.h"
#include "line-editor-uart.h"

#include <string.h>

#define MCODE_GSM_RSP_BUFFER_MAX_LENGTH (80)

static uint32_t TheTimeout = 0;
static size_t TheRspBufferLength = 0;
static char TheRspBuffer[MCODE_GSM_RSP_BUFFER_MAX_LENGTH] = {0};

static void cmd_gsm_mtick(void);
static void cmd_gsm_handle_rsp(char ch);
static void cmd_engine_send_at_command(const char *args);

void cmd_engine_gsm_init(void)
{
  hw_uart2_set_callback(cmd_gsm_handle_rsp);
  mtick_add(cmd_gsm_mtick);
}

void cmd_engine_gsm_deinit(void)
{
  hw_uart2_set_callback(NULL);
}

void cmd_engine_gsm_help(void)
{
  mprintstrln(PSTR("> at <AT-COMMAND> - Send generic AT-command to GSM module"));
}

bool cmd_engine_gsm_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("at "), 3)) {
    cmd_engine_send_at_command(command + 3);
    return true;
  }

  return false;
}

void cmd_engine_send_at_command(const char *args)
{
  mprintstr(PSTR("\r\nSending command: \""));
  mprintstr_R(args);
  mprintstr(PSTR("\"\r\n"));
  char ch;
  while ((ch = *args++)) {
    uart2_write_char(ch);
  }
  uart2_write_char('\r');
}

void cmd_gsm_handle_rsp(char ch)
{
  if (MCODE_GSM_RSP_BUFFER_MAX_LENGTH == TheRspBufferLength) {
    /* Response is too long */
    /** @todo implement handling this case */
    return;
  }

  TheRspBuffer[TheRspBufferLength++] = ch;
  TheTimeout = 100;
}

void cmd_gsm_mtick(void)
{
  if (!TheTimeout || --TheTimeout) {
    return;
  }

  mprintstr(PSTR("\r>>> GSM "));
  mprintstr(PSTR("response: \""));
  mprintbytes(TheRspBuffer, TheRspBufferLength);
  mprintstrln(PSTR("\""));
  line_editor_uart_start();
  memset(TheRspBuffer, 0, sizeof (TheRspBuffer));
  TheRspBufferLength = 0;
}
