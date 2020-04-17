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

void gsm_sms_send_body(void);
void gsm_uart2_handler(const char *data, size_t length);
void gsm_handle_new_sms(const char *args, size_t length);
void gsm_read_sms_handle_body(const char *data, size_t length);
void gsm_read_sms_handle_header(const char *data, size_t length);
void gsm_read_sms_handle_response(const char *data, size_t length);
}

extern "C" TGsmState TheGsmState;
extern "C" TGsmStateFlags TheGsmFlags;

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
    io_set_ostream_handler(alt_uart_write_char);
    collected_text_reset();
    collected_alt_text_reset();
    _event = false;
    _type = MGsmEventNone;
    memset(_from, 0, sizeof (_from));
    memset(_body, 0, sizeof (_body));
  }
  void TearDown() override {
    collected_text_reset();
    collected_alt_text_reset();
    io_set_ostream_handler(NULL);
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

bool GsmBasic::_event = false;
MGsmEvent GsmBasic::_type = MGsmEventNone;
char GsmBasic::_from[100] = {0};
char GsmBasic::_body[512] = {0};

TEST_F(GsmBasic, GsmReadSmsHandleHeader)
{
  bool result;
  const char header[] = "+CMGR: \"REC READ\",\"002B00390038003800370035003300310030003100320033\",\"\",\"20/01/08,10:25:13+12\"";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsHeader;
  gsm_read_sms_handle_header(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateReadingSmsBody);
  ASSERT_EQ(collected_alt_text_length(), 12);
  ASSERT_STREQ(collected_alt_text(), "+98875310123");
}

TEST_F(GsmBasic, GsmReadSmsHandleBody)
{
  bool result;
  const char header[] = "005400650073007400200053004D0053003A00200061006200630064";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsBody;
  gsm_read_sms_handle_body(header, length);

  ASSERT_EQ(TheGsmState, EGsmStateIdle);
  ASSERT_EQ(collected_alt_text_length(), 14);
  ASSERT_STREQ(collected_alt_text(), "Test SMS: abcd");
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
  TheGsmState = EGsmStateIdle;
  TheGsmFlags = EGsmStateFlagAtReady;
  gsm_send_cmd("AT");

  ASSERT_EQ(collected_alt_text_length(), 2);
  ASSERT_STREQ(collected_alt_text(), "AT");
}

TEST_F(GsmBasic, SendCmdRaw)
{
  TheGsmState = EGsmStateIdle;
  TheGsmFlags = EGsmStateFlagAtReady;
  gsm_send_cmd_raw("AT");

  ASSERT_EQ(collected_alt_text_length(), 2);
  ASSERT_STREQ(collected_alt_text(), "AT");
}

void hw_gsm_init(void)
{
  TheHwGsmInitSent = true;
}

void hw_gsm_power(bool on)
{
  TheLastPowerRequest = on;
}
