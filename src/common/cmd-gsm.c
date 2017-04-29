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
#define MCODE_GSM_PHONE_NUMBER_MAX_LENGTH (16)

static uint32_t TheTimeout = 0;
static size_t TheRspBufferLength = 0;
static bool TheWaitingForGsmFlag = false;
static char TheMsgBuffer[140] = {0};
static char TheRspBuffer[MCODE_GSM_RSP_BUFFER_MAX_LENGTH] = {0};
static char TheNumberBuffer[MCODE_GSM_PHONE_NUMBER_MAX_LENGTH] = {0};

static void cmd_gsm_mtick(void);
static void cmd_gsm_handle_rsp(char ch);
static void cmd_gsm_show_phone_number(void);
static void cmd_gsm_send_sms(const char *body);
static void cmd_gsm_send_string(const char *str);
static void cmd_engine_send_at_command(const char *args);
static void cmd_gsm_store_phone_number(const char *number);

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
  mprintstrln(PSTR("> send-sms <MSG-BODY> - Send SMS with <MSG-BODY> text"));
  mprintstrln(PSTR("> phone - Show the current phone number for sending SMSes"));
  mprintstrln(PSTR("> phone-set <PHONE-NUMBER> - Store the phone number for sending SMS"));
}

bool cmd_engine_gsm_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("at "), 3)) {
    cmd_engine_send_at_command(command + 3);
    return true;
  } else if (!strncmp_P(command, PSTR("send-sms "), 9)) {
    cmd_gsm_send_sms(command + 9);
    return true;
  } else if (!strncmp_P(command, PSTR("phone-set "), 10)) {
    cmd_gsm_store_phone_number(command + 10);
    return true;
  } else if (!strcmp_P(command, PSTR("phone"))) {
    cmd_gsm_show_phone_number();
    return true;
  }

  return false;
}

void cmd_engine_send_at_command(const char *args)
{
  mprintstr(PSTR("\r\nSending command: \""));
  mprintstr_R(args);
  mprintstr(PSTR("\"\r\n"));
  cmd_gsm_send_string(args);
  uart2_write_char('\r');
}

void cmd_gsm_send_sms(const char *body)
{
  const size_t length = strlen(body);
  if (length > sizeof (TheMsgBuffer) - 1) {
    mprintstrln(PSTR("Error: the message body is too long"));
    return;
  }
  if (!strlen(TheNumberBuffer)) {
    mprintstrln(PSTR("Error: no phone number, use 'phone-set' first"));
    return;
  }

  memcpy(TheMsgBuffer, body, length);
  cmd_gsm_send_string("AT+CMGS=\"");
  cmd_gsm_send_string(TheNumberBuffer);
  cmd_gsm_send_string(PSTR("\"\r"));
  TheWaitingForGsmFlag = true;
}

void cmd_gsm_store_phone_number(const char *number)
{
  const size_t length = strlen(number);
  if (length > MCODE_GSM_PHONE_NUMBER_MAX_LENGTH - 1) {
    mprintstrln(PSTR("Error: phone number is too long"));
    return;
  }
  strcpy(TheNumberBuffer, number);
}

void cmd_gsm_show_phone_number(void)
{
  mprintstr(PSTR("Current phone number: \""));
  mprintstr_R(TheNumberBuffer);
  mprintstrln(PSTR("\""));
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

  if (TheWaitingForGsmFlag) {
    TheWaitingForGsmFlag = false;
    cmd_gsm_send_string(TheMsgBuffer);
    uart2_write_char(0x1a);
    uart2_write_char('\r');
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

void cmd_gsm_send_string(const char *str)
{
  char ch;
  while ((ch = *str++)) {
    uart2_write_char(ch);
  }
}
