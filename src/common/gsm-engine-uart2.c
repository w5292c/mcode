/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2020 Alexander Chumakov
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

#include "mvars.h"
#include "hw-uart.h"
#include "mglobal.h"
#include "mparser.h"
#include "mstring.h"
#include "line-editor-uart.h"

#include <stddef.h>
#include <string.h>

#define MCODE_SMS_MAX_LENGTH (140)

/*
 * States flow:
 * * Got new SMS notification: store the index in NVM (n0:2) (0-th bit for index '0');
 * * In IDLE state: check NVM (n0:2) for new SMS flags:
 *                  if yes - read SMS and store in s0:1, s1:2;
 * * When the SMS is read to s0:1, s1:2: configure the output to be stored in s3:1
 *   and execute it in Command Engine;
 * * Send s3:1 in SMS to the original address;
 * * Go to IDLE for now, check if we need to delete the incoming SMS;
 */

typedef enum {
  EGsmStateNull = 0,
  EGsmStateIdle,
  EGsmStateSendingAtCmd,
  EGsmStateSendingSmsAddress,
  EGsmStateReadingSmsBody,
  EGsmStateReadingSmsHeader,
} TGsmState;

typedef enum {
  EGsmStateFlagNone = 0,
  EGsmStateFlagAtReady = 1,
  EGsmStateFlagSmsReady = 2,
  EGsmStateFlagCallReady = 4,
  EGsmStateFlagPinReady = 8,
  EGsmStateFlagAllReady = 15,
} TGsmStateFlags;

typedef enum {
  EAtCmdIdUnknown = -1,
  EAtCmdIdNull = 0,
  EAtCmdIdOk,
  EAtCmdIdError,
  EAtCmdIdEmpty,
  EAtCmdIdReady,
  EAtCmdIdSmsRead,
  EAtCmdIdSmsSent,
  EAtCmdIdFullFunc,
  EAtCmdIdSmsReady,
  EAtCmdIdSmsIndication,
  EAtCmdIdPinReady,
  EAtCmdIdPinNotReady,
  EAtCmdIdCallReady,
  EAtCmdIdBatteryLevel,
  EAtCmdIdSmsReadyForBody,
} TAtCmdId;

typedef struct {
  const char *rspBase;
  TAtCmdId cmdId;
} TGsmResponses;

static const TGsmResponses TheGsmResponses[] = {
  { "", EAtCmdIdEmpty },
  { "OK", EAtCmdIdOk },
  { "ERROR", EAtCmdIdError },
  { "RDY", EAtCmdIdReady },
  { "SMS Ready", EAtCmdIdSmsReady },
  { "Call Ready", EAtCmdIdCallReady },
  { "+CPIN: READY", EAtCmdIdPinReady },
  { "+CPIN: NOT READY", EAtCmdIdPinNotReady},
  { "+CFUN: 1", EAtCmdIdFullFunc },
  { "+CBC", EAtCmdIdBatteryLevel },
  { "+CMGS", EAtCmdIdSmsSent },
  { "+CMTI", EAtCmdIdSmsIndication },
  { "+CMGR", EAtCmdIdSmsRead },
  { "> ", EAtCmdIdSmsReadyForBody },
  { NULL, EAtCmdIdNull },
};

static gsm_callback TheGsmCallback = NULL;
static TGsmState TheGsmState = EGsmStateNull;
static char TheMsgBuffer[MCODE_SMS_MAX_LENGTH] = {0};
static TGsmStateFlags TheGsmFlags = EGsmStateFlagNone;

static void gsm_sms_send_body(void);
static void gsm_uart2_handler(const char *data, size_t length);
static void gsm_handle_new_sms(const char *args, size_t length);
static void gsm_read_sms_handle_body(const char *data, size_t length);
static void gsm_read_sms_handle_header(const char *data, size_t length);
static void gsm_read_sms_handle_response(const char *data, size_t length);
static const char *gsm_parse_response(const char *rsp, TAtCmdId *id, const char **args);

void gsm_init(void)
{
  if (EGsmStateNull != TheGsmState) {
    /* Already initialized */
    return;
  }

  hw_gsm_init();

  TheGsmState = EGsmStateNull;

  hw_uart2_set_callback(gsm_uart2_handler);
}

void gsm_deinit(void)
{
  if (EGsmStateNull == TheGsmState) {
    /* Nothing to deinitialize */
    return;
  }

  hw_uart2_set_callback(NULL);
  TheGsmState = EGsmStateNull;
}

void gsm_set_callback(gsm_callback callback)
{
  TheGsmCallback = callback;
}

void gsm_power(bool on)
{
  hw_gsm_power(on);
}

bool gsm_send_cmd(const char *cmd)
{
  if (EGsmStateIdle != TheGsmState ||
      0 == (TheGsmFlags & EGsmStateFlagAtReady)) {
    /* GSM engine is not ready */
    return false;
  }

  TheGsmState = EGsmStateSendingAtCmd;

  io_ostream_handler_push(uart2_write_char);
  mprintexpr(cmd, -1);
  mputch('\r');
  io_ostream_handler_pop();

  return true;
}

void gsm_send_cmd_raw(const char *cmd)
{
  /* No state check for RAW variant */
  io_ostream_handler_push(uart2_write_char);
  mprintexpr(cmd, -1);
  mputch('\r');
  io_ostream_handler_pop();
}

bool gsm_send_sms(const char *address, const char *body)
{
  if (EGsmStateIdle != TheGsmState ||
      0 == (TheGsmFlags & EGsmStateFlagAtReady) ||
      0 == (TheGsmFlags & EGsmStateFlagSmsReady) ||
      0 == (TheGsmFlags & EGsmStateFlagPinReady)) {
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
  io_ostream_handler_push(uart2_write_char);
  mprintstr(PSTR("AT+CMGS=\""));
  mprintstrhex16encoded(address, strlen(address));
  mputch('\r');
  io_ostream_handler_pop();
  TheGsmState = EGsmStateSendingSmsAddress;

  return true;
}

void gsm_uart2_handler(const char *data, size_t length)
{
  TAtCmdId id;
  const char *args = NULL;
  const char *next = data;
  const char *curr = data;

  if (EGsmStateReadingSmsBody == TheGsmState ||
      EGsmStateReadingSmsHeader == TheGsmState) {
    gsm_read_sms_handle_response(data, length);
    return;
  }
  while (next) {
    next = gsm_parse_response(next, &id, &args);
    switch (id) {
    case EAtCmdIdNull:
    case EAtCmdIdEmpty:
      break;
    case EAtCmdIdOk:
      mprintstrln(PSTR("\r- OK event"));
      if (EGsmStateSendingAtCmd == TheGsmState) {
        TheGsmState = EGsmStateIdle;
      }
      break;
    case EAtCmdIdError:
      mprintstrln(PSTR("\r- ERROR event"));
      if (EGsmStateSendingAtCmd == TheGsmState) {
        TheGsmState = EGsmStateIdle;
      }
      break;
    case EAtCmdIdReady:
      TheGsmFlags |= EGsmStateFlagAtReady;
      mprintstrln(PSTR("\r- READY event"));
      if (EGsmStateNull == TheGsmState) {
        TheGsmState = EGsmStateIdle;
      }
      break;
    case EAtCmdIdSmsReady:
      TheGsmFlags |= EGsmStateFlagSmsReady;
      mprintstrln(PSTR("\r- SMS-READY event"));
      break;
    case EAtCmdIdCallReady:
      TheGsmFlags |= EGsmStateFlagCallReady;
      mprintstrln(PSTR("\r- Call-READY event"));
      break;
    case EAtCmdIdPinReady:
      TheGsmFlags |= EGsmStateFlagPinReady;
      mprintstrln(PSTR("\r- PIN-READY event"));
      break;
    case EAtCmdIdPinNotReady:
      TheGsmFlags = EGsmStateFlagAtReady;
      mprintstrln(PSTR("\r- PIN-NOT-READY event"));
      break;
    case EAtCmdIdFullFunc:
      mprintstrln(PSTR("\r- Full-functionality event"));
      break;
    case EAtCmdIdBatteryLevel:
      mprintstr(PSTR("\r- Battery level event, args: "));
      if (args) {
        mprintstr(PSTR("\""));
        mprintbytes(args, next - args - 1);
        mprintstrln(PSTR("\""));
      } else {
        mprintstrln(PSTR("<null>"));
      }
      break;
    case EAtCmdIdSmsSent:
      mprintstr(PSTR("\r- SMS sent event, args: "));
      if (args) {
        mprintstr(PSTR("\""));
        mprintbytes(args, next - args - 1);
        mprintstrln(PSTR("\""));
      } else {
        mprintstrln(PSTR("<null>"));
      }
      break;
    case EAtCmdIdSmsReadyForBody:
      if (EGsmStateSendingSmsAddress == TheGsmState) {
        mprintstrln(PSTR("\r- SMS: ready for body event"));
        gsm_sms_send_body();
        TheGsmState = EGsmStateSendingAtCmd;
      } else {
        mprintstrln(PSTR("\r- Error: unexpected ready for body event"));
      }
      break;
    case EAtCmdIdSmsIndication:
      gsm_handle_new_sms(args, next - args - 1);
      break;
    default:
    case EAtCmdIdUnknown:
      mprintstr(PSTR("\r- Unknown event: \""));
      mprintbytes(curr, next - curr - 1);
      mprintstrln(PSTR("\""));
      break;
    }
    curr = next;
  };

  mprintstr(PSTR("\r"));
  line_editor_uart_start();
}

void gsm_sms_send_body(void)
{
  TheGsmState = EGsmStateIdle;
  io_ostream_handler_push(uart2_write_char);
  mprintstrhex16encoded(TheMsgBuffer, strlen(TheMsgBuffer));
  mputch('\e');
  mputch('\r');
  io_ostream_handler_pop();

  memset(TheMsgBuffer, 0, sizeof (TheMsgBuffer));
}

const char *gsm_parse_response(const char *rsp, TAtCmdId *id, const char **args)
{
  if (!rsp) {
    *id = EAtCmdIdNull;
    return NULL;
  }

  const char *nextLine = rsp;
  do {
    const char ch = *nextLine++;
    if (!ch) {
      nextLine = NULL;
      break;
    }
    if ('\n' == ch || '\r' == ch) {
      break;
    }
  } while (true);

  const size_t lineLength = nextLine ? (nextLine - rsp - 1) : strlen(rsp);

  const char *itemArgs = NULL;
  TAtCmdId itemId = EAtCmdIdUnknown;
  const TGsmResponses *response = TheGsmResponses;
  while (response->rspBase) {
    /* First, try the full match */
    const size_t itemLength = strlen(response->rspBase);
    if (lineLength == itemLength && !strncmp(response->rspBase, rsp, lineLength)) {
      itemId = response->cmdId;
      break;
    }

    /* Now, try partial match, like '+CMGS: <any>', @todo implement */
    if ('+' == *response->rspBase && !strchr(response->rspBase, ':')) {
      const char *marker = (const char *)memchr(rsp, ':', lineLength);
      if (marker) {
        const size_t tokenLength = marker - rsp;
        if (tokenLength == itemLength && !strncmp(response->rspBase, rsp, itemLength)) {
          itemId = response->cmdId;
          itemArgs = rsp + tokenLength + 2;
          break;
        }
      }
    }

    /* Try the next item */
    ++response;
  }

  *args = itemArgs;
  *id = itemId;
  return nextLine;
}

void gsm_handle_new_sms(const char *args, size_t length)
{
  mprintstr(PSTR("\r- New SMS event, args: "));
  if (args) {
    mprintstr(PSTR("\""));
    mprintbytes(args, length);
    mprintstrln(PSTR("\""));
  } else {
    mprintstrln(PSTR("<null>"));
  }
}

bool gsm_read_sms(int index)
{
  if (EGsmStateIdle != TheGsmState) {
    /* GSM engine is not ready */
    return false;
  }

  TheGsmState = EGsmStateReadingSmsHeader;
  io_ostream_handler_push(uart2_write_char);
  mprintstr(PSTR("AT+CMGR="));
  mprint_uintd(index, 1);
  mputch('\r');
  io_ostream_handler_pop();

  return true;
}

void gsm_read_sms_handle_response(const char *data, size_t length)
{
  if (EGsmStateReadingSmsHeader == TheGsmState) {
    gsm_read_sms_handle_header(data, length);
  } else if (EGsmStateReadingSmsBody == TheGsmState) {
    gsm_read_sms_handle_body(data, length);
  }
}

void gsm_read_sms_handle_header(const char *data, size_t length)
{
  uint32_t value;
  TokenType type;
  const char *token;
  /*
   * Example response:
   * > +CMGR: "REC READ","002B00390038003800370035003300310030003100320033","","20/01/08,10:25:13+12""
   */

  do {
    /* Parse '+' */
    type = next_token(&data, &length, &token, &value);
    if (TokenEnd == type) {
      /* Got empty notification, skip it */
      return;
    } else if (TokenPunct != type || '+' != value) {
      /* Failed parsing */
      break;
    }
    /* Parse "CMGR:" */
    type = next_token(&data, &length, &token, &value);
    if (TokenId != type || !mparser_strcmp(token, length, "CMGR:")) {
      break;
    }
    /* Parse whitespace after ':' */
    type = next_token(&data, &length, &token, &value);
    if (TokenWhitespace != type || ' ' != value) {
      break;
    }
    /* Parse "REC READ" */
    type = next_token(&data, &length, &token, &value);
    if (TokenString != type) {
      break;
    }
    /* Parse ',' */
    type = next_token(&data, &length, &token, &value);
    if (TokenPunct != type || ',' != value) {
      break;
    }
    /* Parse the phone number field */
    type = next_token(&data, &length, &token, &value);
    if (TokenString != type) {
      break;
    }
    /* At this point we have the phone number UCS2-encoded in 'token'/'value'(length)
     * Need to check the phone number at this point */
    mvar_putch_config(0, 1);
    io_ostream_handler_push(mvar_putch);
    mprinthexencodedstr16(token, value);
    io_ostream_handler_pop();

    /* Wait for the SMS body */
    TheGsmState = EGsmStateReadingSmsBody;
    return;
  } while (false);
  TheGsmState = EGsmStateIdle;
}

void gsm_read_sms_handle_body(const char *data, size_t length)
{
  /*
   * Example body:
   * > 005400650073007400200053004D0053003A00200061006200630064
   */
  mvar_putch_config(1, 2);
  io_ostream_handler_push(mvar_putch);
  mprinthexencodedstr16(data, length);
  io_ostream_handler_pop();

  /* Finished handling the SMS */
  TheGsmState = EGsmStateIdle;
}
