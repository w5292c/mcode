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

#include "hw-uart.h"
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

void gsm_sms_send_body(void);
void gsm_send_string(const char *str);
void gsm_send_fstring(const char *str);
void gsm_uart2_handler(const char *data, size_t length);
void gsm_handle_new_sms(const char *args, size_t length);
void gsm_read_sms_handle_body(const char *data, size_t length);
void gsm_read_sms_handle_header(const char *data, size_t length);
void gsm_read_sms_handle_response(const char *data, size_t length);
//const char *gsm_parse_response(const char *rsp, TAtCmdId *id, const char **args);

extern "C" TGsmState TheGsmState;
}

using namespace testing;

class GsmBasic : public Test
{
protected:
  void SetUp() override {
    collected_text_reset();
    collected_alt_text_reset();
  }
  void TearDown() override {
    collected_text_reset();
    collected_alt_text_reset();
  }
};

TEST_F(GsmBasic, GsmReadSmsHandleHeader)
{
  bool result;
  const char header[] = "+CMGR: \"REC READ\",\"002B00390038003800370035003300310030003100320033\",\"\",\"20/01/08,10:25:13+12\"";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsHeader;
  gsm_read_sms_handle_header(header, length);
  ASSERT_EQ(TheGsmState, EGsmStateReadingSmsBody);

  ASSERT_EQ(collected_text_length(), 21);
  ASSERT_STREQ(collected_text(), "Phone: +98875310123\r\n");
}

TEST_F(GsmBasic, GsmReadSmsHandleBody)
{
  bool result;
  const char header[] = "005400650073007400200053004D0053003A00200061006200630064";
  size_t length = sizeof (header) - 1;

  TheGsmState = EGsmStateReadingSmsBody;
  gsm_read_sms_handle_body(header, length);
  ASSERT_EQ(TheGsmState, EGsmStateIdle);

  ASSERT_EQ(collected_text_length(), 27);
  ASSERT_STREQ(collected_text(), "SMS body [Test SMS: abcd]\r\n");
}

void hw_gsm_init(void)
{
}

void hw_gsm_power(bool on)
{
}

void hw_uart2_set_callback(hw_uart_handler cb)
{
}