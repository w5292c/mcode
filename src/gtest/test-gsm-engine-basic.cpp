/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Alexander Chumakov
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
#include "mstring.h"
#include "wrap-mocks.h"

#include <gtest/gtest.h>

#define MCODE_SMS_MAX_LENGTH (140)

extern "C" {
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

void gsm_sms_send_body(void);
void gsm_uart2_handler(const char *data, size_t length);
void gsm_handle_new_sms(const char *args, size_t length);
void gsm_read_sms_handle_body(const char *data, size_t length);
void gsm_read_sms_handle_header(const char *data, size_t length);
void gsm_read_sms_handle_response(const char *data, size_t length);
const char *gsm_parse_response(const char *rsp, TAtCmdId *id, const char **args);
}

extern "C" TGsmState TheGsmState;
extern "C" TGsmStateFlags TheGsmFlags;
extern "C" char TheMsgBuffer[MCODE_SMS_MAX_LENGTH];

static bool TheHwGsmInitSent = false;
static bool TheLastPowerRequest = false;

using namespace testing;

class GsmBasic : public Test
{
protected:
  void SetUp() override {
    TheHwGsmInitSent = false;
    TheLastPowerRequest = false;
    gsm_init();
    gsm_set_callback(gsm_callback);
    TheGsmState = EGsmStateIdle;
    TheGsmFlags = EGsmStateFlagAtReady;
    collected_text_reset();
    collected_text2_reset();
    collected_alt_text_reset();
    _event = false;
    _type = MGsmEventNone;
    memset(_from, 0, sizeof (_from));
    memset(_body, 0, sizeof (_body));
    // Define the default handler
    io_ostream_handler_push(alt_uart_write_char);
  }
  void TearDown() override {
    io_ostream_handler_pop();
    collected_text_reset();
    collected_alt_text_reset();
    gsm_set_callback(NULL);
    gsm_deinit();
  }
  static void gsm_callback(MGsmEvent type, const char *from, const char *body) {
    _event = true;
    _type = type;
    strcpy(_from, from);
    strcpy(_body, body);
  }

  static bool _event;
  static MGsmEvent _type;
  static char _from[100];
  static char _body[512];
};

class SmsReadHandling : public Test
{
protected:
  void SetUp() override {
    collected_text2_reset();
    char *str;
    size_t length;
    // Reset 's0:1' variable
    str = mvar_str(0, 1, &length);
    memset(str, 0, length);
    // Reset 's1:2' variable
    str = mvar_str(1, 2, &length);
    memset(str, 0, length);
    gsm_init();
    TheGsmState = EGsmStateIdle;
    TheGsmFlags = EGsmStateFlagAtReady;
    // Define the default handler
    io_ostream_handler_push(alt_uart_write_char);
  }
  void TearDown() override {
    io_ostream_handler_pop();
    gsm_deinit();
  }

  const char *phone_var_str() {
    return mvar_str(0, 1, NULL);
  }
  size_t phone_var_length() {
    return strlen(phone_var_str());
  }
  const char *body_var_str() {
    return mvar_str(1, 2, NULL);
  }
  size_t body_var_length() {
    return strlen(body_var_str());
  }
};

bool GsmBasic::_event = false;
MGsmEvent GsmBasic::_type = MGsmEventNone;
char GsmBasic::_from[100] = {0};
char GsmBasic::_body[512] = {0};

TEST_F(SmsReadHandling, GsmReadSmsHandleHeader)
{
  bool result;
  const char header[] = "+CMGR: \"REC READ\",\"002B00390038003800370035003300310030003100320033\",\"\",\"20/01/08,10:25:13+12\"";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsHeader;
  gsm_read_sms_handle_header(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateReadingSmsBody);
  ASSERT_EQ(phone_var_length(), 12);
  ASSERT_STREQ(phone_var_str(), "+98875310123");
}

TEST_F(SmsReadHandling, GsmReadSmsResponseHandleHeader)
{
  bool result;
  const char header[] = "+CMGR: \"REC READ\",\"002B00390038003800370035003300310030003100320033\",\"\",\"20/01/08,10:25:13+12\"";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsHeader;
  gsm_read_sms_handle_response(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateReadingSmsBody);
  ASSERT_EQ(phone_var_length(), 12);
  ASSERT_STREQ(phone_var_str(), "+98875310123");
}

TEST_F(SmsReadHandling, GsmReadSmsUart2ResponseHandleHeader)
{
  bool result;
  const char header[] = "+CMGR: \"REC READ\",\"002B00390038003800370035003300310030003100320033\",\"\",\"20/01/08,10:25:13+12\"";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsHeader;
  gsm_uart2_handler(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateReadingSmsBody);
  ASSERT_EQ(phone_var_length(), 12);
  ASSERT_STREQ(phone_var_str(), "+98875310123");
}

TEST_F(SmsReadHandling, GsmReadSmsHandleBody)
{
  bool result;
  const char header[] = "005400650073007400200053004D0053003A00200061006200630064";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsBody;
  gsm_read_sms_handle_body(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
  ASSERT_EQ(body_var_length(), 14);
  ASSERT_STREQ(body_var_str(), "Test SMS: abcd");
}

TEST_F(SmsReadHandling, GsmReadSmsResponseHandleBody)
{
  bool result;
  const char header[] = "005400650073007400200053004D0053003A00200061006200630064";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsBody;
  gsm_read_sms_handle_response(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
  ASSERT_EQ(body_var_length(), 14);
  ASSERT_STREQ(body_var_str(), "Test SMS: abcd");
}

TEST_F(SmsReadHandling, GsmReadUart2ResponseHandleBody)
{
  bool result;
  const char header[] = "005400650073007400200053004D0053003A00200061006200630064";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsBody;
  gsm_uart2_handler(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
  ASSERT_EQ(body_var_length(), 14);
  ASSERT_STREQ(body_var_str(), "Test SMS: abcd");
}

TEST_F(GsmBasic, DoubleInit)
{
  gsm_init();
  gsm_init();
}

TEST_F(GsmBasic, DoubleDeInit)
{
  gsm_deinit();
  gsm_deinit();
}

TEST_F(GsmBasic, PowerOn)
{
  TheLastPowerRequest = false;
  gsm_power(true);

  ASSERT_TRUE(TheLastPowerRequest);
}

TEST_F(GsmBasic, PowerOff)
{
  TheLastPowerRequest = true;
  gsm_power(false);

  ASSERT_FALSE(TheLastPowerRequest);
}

TEST_F(GsmBasic, SendCmd)
{
  gsm_send_cmd("AT");

  ASSERT_EQ(collected_text2_length(), 3);
  ASSERT_STREQ(collected_text2(), "AT\r");
}

TEST_F(GsmBasic, SendCmdRaw)
{
  gsm_send_cmd_raw("ATD");

  ASSERT_EQ(collected_text2_length(), 4);
  ASSERT_STREQ(collected_text2(), "ATD\r");
}

TEST_F(SmsReadHandling, GsmReadSmsPositive)
{
  const bool res = gsm_read_sms(7);

  ASSERT_TRUE(res);
  ASSERT_EQ(collected_text2_length(), 10);
  ASSERT_STREQ(collected_text2(), "AT+CMGR=7\r");
}

TEST_F(SmsReadHandling, GsmReadSmsNegative)
{
  TheGsmState = EGsmStateReadingSmsHeader;
  const bool res = gsm_read_sms(7);

  ASSERT_FALSE(res);
}

TEST_F(SmsReadHandling, GsmSendSmsPositive)
{
  TheGsmState = EGsmStateIdle;
  TheGsmFlags = (TGsmStateFlags)(EGsmStateFlagAtReady |
                                 EGsmStateFlagSmsReady | EGsmStateFlagPinReady);

  bool res = gsm_send_sms("+70001112233", "ABCD");
  ASSERT_TRUE(res);
  ASSERT_STREQ(TheMsgBuffer, "ABCD");
  ASSERT_EQ(collected_text2_length(), 59);
  ASSERT_EQ(TheGsmState, EGsmStateSendingSmsAddress);
  ASSERT_STREQ(collected_text2(), "AT+CMGS=\"002B00370030003000300031003100310032003200330033\"\r");
}

TEST_F(SmsReadHandling, GsmSendSmsBodyPositive)
{
  memset(TheMsgBuffer, 0, sizeof (TheMsgBuffer));
  strcpy(TheMsgBuffer, "ABCD");

  gsm_sms_send_body();

  ASSERT_EQ(collected_text2_length(), 18);
  ASSERT_EQ(TheGsmState, EGsmStateIdle);
  ASSERT_STREQ(collected_text2(), "0041004200430044\x1a\r");
}

TEST_F(SmsReadHandling, GsmSendModemRspNull)
{
  TheGsmState = EGsmStateIdle;

  gsm_uart2_handler(NULL, 0);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspEmpty)
{
  TheGsmState = EGsmStateIdle;

  gsm_uart2_handler("", 0);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspOk)
{
  TheGsmState = EGsmStateSendingAtCmd;

  gsm_uart2_handler("OK", 2);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspError)
{
  TheGsmState = EGsmStateSendingAtCmd;

  gsm_uart2_handler("ERROR", 5);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspReady)
{
  TheGsmState = EGsmStateNull;
  TheGsmFlags = EGsmStateFlagNone;

  gsm_uart2_handler("RDY", 3);

  ASSERT_EQ(TheGsmFlags&EGsmStateFlagAtReady, EGsmStateFlagAtReady);
  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspSmsReady)
{
  TheGsmFlags = EGsmStateFlagNone;

  gsm_uart2_handler("SMS Ready", 9);

  ASSERT_EQ(TheGsmFlags&EGsmStateFlagSmsReady, EGsmStateFlagSmsReady);
}

TEST_F(SmsReadHandling, GsmSendModemRspCallReady)
{
  TheGsmFlags = EGsmStateFlagNone;

  gsm_uart2_handler("Call Ready", 10);

  ASSERT_EQ(TheGsmFlags&EGsmStateFlagCallReady, EGsmStateFlagCallReady);
}

TEST_F(SmsReadHandling, GsmSendModemRspPinReady)
{
  TheGsmFlags = EGsmStateFlagNone;

  gsm_uart2_handler("+CPIN: READY", 12);

  ASSERT_EQ(TheGsmFlags&EGsmStateFlagPinReady, EGsmStateFlagPinReady);
}

TEST_F(SmsReadHandling, GsmSendModemRspPinNotReady)
{
  TheGsmFlags = EGsmStateFlagAllReady;

  gsm_uart2_handler("+CPIN: NOT READY", 16);

  ASSERT_EQ(TheGsmFlags, EGsmStateFlagAtReady);
}

TEST_F(SmsReadHandling, GsmSendModemRspFullFunc)
{
  TheGsmFlags = EGsmStateFlagNone;

  gsm_uart2_handler("+CFUN: 1", 8);
}

TEST_F(SmsReadHandling, GsmSendModemRspBatteryLevel)
{
  gsm_uart2_handler("+CBC: 5", 7);
}

TEST_F(SmsReadHandling, GsmSendModemRspBatteryLevelNoInfo)
{
  gsm_uart2_handler("+CBC", 4);
}

TEST_F(SmsReadHandling, GsmSendModemRspSmsSent)
{
  gsm_uart2_handler("+CMGS: 5", 8);
}

TEST_F(SmsReadHandling, GsmSendModemRspSmsSentNoInfo)
{
  gsm_uart2_handler("+CMGS", 5);
}

TEST_F(SmsReadHandling, GsmSendModemRspReadyForBody)
{
  memset(TheMsgBuffer, 0, sizeof (TheMsgBuffer));
  strcpy(TheMsgBuffer, "ABCD");

  TheGsmState = EGsmStateSendingSmsAddress;
  gsm_uart2_handler("> ", 2);

  ASSERT_EQ(collected_text2_length(), 18);
  ASSERT_EQ(TheGsmState, EGsmStateSendingAtCmd);
  ASSERT_STREQ(collected_text2(), "0041004200430044\x1a\r");
}

TEST_F(SmsReadHandling, GsmSendModemRspReadyForBodyNegative)
{
  memset(TheMsgBuffer, 0, sizeof (TheMsgBuffer));
  strcpy(TheMsgBuffer, "ABCD");

  TheGsmState = EGsmStateIdle;
  gsm_uart2_handler("> ", 2);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspUnknownReponse)
{
  TheGsmState = EGsmStateIdle;

  gsm_uart2_handler("AT+CUNKNOWN", 11);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspNewSmsIndication)
{
  TheGsmState = EGsmStateIdle;

  gsm_uart2_handler("+CMTI: \"SM\",3", 13);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmSendModemRspNewSmsIndicationNoInfo)
{
  TheGsmState = EGsmStateIdle;

  gsm_uart2_handler("+CMTI", 5);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
}

TEST_F(SmsReadHandling, GsmParseReponseNullReponse)
{
  TAtCmdId cmd_id;
  gsm_parse_response(NULL, &cmd_id, NULL);
}

void hw_gsm_init(void)
{
  TheHwGsmInitSent = true;
}

void hw_gsm_power(bool on)
{
  TheLastPowerRequest = on;
}
