/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Alexander Chumakov
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

#include "hw-uart.h"
#include "mglobal.h"
#include "mstring.h"
#include "line-editor-uart.h"

#include <stddef.h>
#include <string.h>

#define MCODE_SMS_MAX_LENGTH (140)

typedef enum {
  EGsmStateNull = 0,
  EGsmStateIdle,
  EGsmStateSendingSmsAddress,
} TGsmState;

static gsm_callback TheGsmCallback = NULL;
static TGsmState TheGsmState = EGsmStateNull;
static char TheMsgBuffer[MCODE_SMS_MAX_LENGTH] = {0};

static void gsm_send_string(const char *str);
static void gsm_uart2_handler(char *data, size_t length);

void gsm_init(void)
{
  if (EGsmStateNull != TheGsmState) {
    /* Already initialized */
    return;
  }

  /** @todo Wait for 'RDY' before going to idle */
  TheGsmState = EGsmStateIdle;

  hw_uart2_set_callback(gsm_uart2_handler);
}

void gsm_deinit(void)
{
  if (EGsmStateNull == TheGsmState) {
    /* Nothing to deinitialize */
    return;
  }

  hw_uart2_set_callback(NULL);
}

void gsm_set_callback(gsm_callback callback)
{
  TheGsmCallback = callback;
}

void gsm_send_cmd(const char *cmd)
{
  gsm_send_string(cmd);
  uart2_write_char('\r');
}

bool gsm_send_sms(const char *address, const char *body)
{
  if (EGsmStateIdle != TheGsmState) {
    /* GSM engine is not ready */
    return false;
  }

  /* Now, check/store the SMS body */
  size_t length = strlen(body);
  if (length > sizeof (TheMsgBuffer) - 1) {
    length = sizeof (TheMsgBuffer) - 1;
  }
  memcpy(TheMsgBuffer, body, length);
  TheMsgBuffer[length] = 0;

  /* Send first line of '+CMGS' command */
  gsm_send_string("AT+CMGS=\"");
  gsm_send_string(address);
  gsm_send_string(PSTR("\"\r"));
  TheGsmState = EGsmStateSendingSmsAddress;

  return true;
}

void gsm_uart2_handler(char *data, size_t length)
{
  if (EGsmStateSendingSmsAddress == TheGsmState) {
    TheGsmState = EGsmStateIdle;
    gsm_send_string(TheMsgBuffer);
    uart2_write_char(0x1a);
    uart2_write_char('\r');
    return;
  }

  mprintstr(PSTR("\r>>> GSM "));
  mprintstr(PSTR("response: \""));
  mprintbytes(data, length);
  mprintstrln(PSTR("\""));
  line_editor_uart_start();
}

void gsm_send_string(const char *str)
{
  char ch;
  while ((ch = *str++)) {
    uart2_write_char(ch);
  }
}
