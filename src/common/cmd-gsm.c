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

#include "cmd-engine.h"

#include "mglobal.h"
#include "mparser.h"
#include "mstring.h"
#include "gsm-engine.h"
#include "line-editor-uart.h"

#include <string.h>

#define MCODE_GSM_RSP_BUFFER_MAX_LENGTH (80)
#define MCODE_GSM_PHONE_NUMBER_MAX_LENGTH (16)

static char TheNumberBuffer[MCODE_GSM_PHONE_NUMBER_MAX_LENGTH] = {0};

static void cmd_gsm_show_phone_number(void);
static void cmd_gsm_power(const char *args);
static void cmd_gsm_send_sms(const char *body);
static void cmd_engine_send_at_command(const char *args);
static void cmd_gsm_store_phone_number(const char *number);
static void cmd_engine_send_raw_at_command(const char *args);
static void cmd_gsm_read_sms(const char *body, bool *startCmd);
static void cmd_gsm_event_handler(MGsmEvent type, const char *from, const char *body);

void cmd_engine_gsm_init(void)
{
  gsm_set_callback(cmd_gsm_event_handler);

#ifdef __linux__
  /* Set the test phone number for Emulator target */
  cmd_gsm_store_phone_number("+70001112233");
#endif /* __linux__ */
}

void cmd_engine_gsm_deinit(void)
{
  gsm_set_callback(NULL);
}

void cmd_engine_gsm_help(void)
{
  mprintstrln(PSTR("> gsm-power <on/off> - Turn GSM power ON/OFF"));
  mprintstrln(PSTR("> at <AT-COMMAND> - Send generic AT-command to GSM module"));
  mprintstrln(PSTR("> sms-read <idx> - Read SMS at <idx>"));
  mprintstrln(PSTR("> send-sms <MSG-BODY> - Send SMS with <MSG-BODY> text"));
  mprintstrln(PSTR("> phone - Show the current phone number for sending SMSes"));
  mprintstrln(PSTR("> phone-set <PHONE-NUMBER> - Store the phone number for sending SMS"));
}

bool cmd_engine_gsm_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("at "), 3)) {
    cmd_engine_send_at_command(command + 3);
    return true;
  } else if (!strncmp_P(command, PSTR("rat "), 4)) {
    cmd_engine_send_raw_at_command(command + 4);
    return true;
  } else if (!strncmp_P(command, PSTR("sms-read "), 9)) {
    cmd_gsm_read_sms(command + 9, startCmd);
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
  } else if (!strncmp_P(command, PSTR("gsm-power "), 10)) {
    cmd_gsm_power(command + 10);
    return true;
  }

  return false;
}

void cmd_engine_send_at_command(const char *args)
{
  if (!gsm_send_cmd(args)) {
    mprintstr(PSTR("Error: failed sanding AT command: \""));
    mprintstr_R(args);
    mprintstrln(PSTR("\""));
  }
}

void cmd_engine_send_raw_at_command(const char *args)
{
  gsm_send_cmd_raw(args);
}

void cmd_gsm_read_sms(const char *body, bool *startCmd)
{
  TokenType type;
  uint32_t value;
  const char *token;
  size_t length = strlen(body);

  type = next_token(&body, &length, &token, &value);
  if (TokenInt != type) {
    merror(MStringWrongArgument);
    return;
  }
  mprintstr("Reading SMS at index: ");
  mprint_uintd(value, 1);
  mprint(MStringNewLine);

  if (!gsm_read_sms(value)) {
    merror(MStringInternalError);
  } else {
    startCmd = false;
  }
}

void cmd_gsm_send_sms(const char *body)
{
  if (!strlen(TheNumberBuffer)) {
    mprintstrln(PSTR("Error: no phone number, use 'phone-set' first"));
    return;
  }

  if (!gsm_send_sms(TheNumberBuffer, body)) {
    mprintstrln(PSTR("Error: failed sanding SMS"));
  }
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

void cmd_gsm_event_handler(MGsmEvent type, const char *from, const char *body)
{
  mprintstr(PSTR("\r>>> GSM event ("));
  mprintstr(PSTR(")"));
  if (from && strlen(from)) {
    mprintstr(PSTR(", from: \""));
    mprintstr_R(from);
    mprintstr(PSTR("\", "));
  }
  mprintstr(PSTR("body:\r\n"));
  if (body) {
    mprintstr_R(body);
  } else {
    mprintstr(PSTR("<empty>"));
  }
  mprint(MStringNewLine);
  line_editor_uart_start();
}

void cmd_gsm_power(const char *args)
{
  if (!strcmp_P(args, PSTR("on"))) {
    gsm_power(true);
  } else if (!strcmp_P(args, PSTR("off"))) {
    gsm_power(false);
  }
}
