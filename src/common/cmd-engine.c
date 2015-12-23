/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Alexander Chumakov
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

#include "pwm.h"
#include "mtick.h"
#include "utils.h"
#include "hw-lcd.h"
#include "mglobal.h"
#include "hw-leds.h"
#include "hw-uart.h"
#include "cmd-test-image.h"
#include "line-editor-uart.h"
#include "persistent-store.h"

#include <string.h>

#ifdef MCODE_PWM
static void cmd_engine_pwm(const char *cmd);
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
static void cmd_engine_set_led(const char *cmd);
#endif /* MCODE_LEDS */
static void cmd_engine_on_cmd_ready (const char *aString);
#ifdef MCODE_SECURITY
#include "sha.h"
#endif /* MCODE_SECURITY */
#ifdef MCODE_COMMAND_MODES
typedef enum {
  CommandEngineStateCmd,
  CommandEngineStateArgs,
  CommandEngineStatePass,
} CommandEngineState;

static uint8_t TheMode;
static uint32_t TheSuperUserTimeout;
static uint8_t TheCommandEngineState = CommandEngineStateCmd;
static uint8_t TheCommandEngineStateRequest;
static void cmd_engine_mtick(void);
static void cmd_engine_passwd(void);
static void cmd_engine_set_cmd_mode(const char *params);
static void cmd_engine_handle_args(const char *args);
static void cmd_engine_handle_pass(const char *pass);
#endif /* MCODE_COMMAND_MODES */

void cmd_engine_init (void)
{
  line_editor_uart_init();
  line_editor_uart_set_callback(cmd_engine_on_cmd_ready);
#ifdef MCODE_COMMAND_MODES
  TheMode = CmdModeNormal;
  TheSuperUserTimeout = 0;
  mtick_add(cmd_engine_mtick);
#endif /* MCODE_COMMAND_MODES */
}

void cmd_engine_deinit (void)
{
  line_editor_uart_deinit();
}

void cmd_engine_start (void)
{
  line_editor_uart_start();
}

void cmd_engine_on_cmd_ready (const char *aString)
{
#ifdef MCODE_COMMAND_MODES
  switch (TheCommandEngineState) {
  case CommandEngineStateCmd:
    break;
  case CommandEngineStateArgs:
    cmd_engine_handle_args(aString);
    return;
  case CommandEngineStatePass:
    cmd_engine_handle_pass(aString);
    return;
  default:
    hw_uart_write_string_P(PSTR("Error: wrong engine state: "));
    hw_uart_write_uint(TheCommandEngineState);
    hw_uart_write_string_P(PSTR("\r\n"));
    return;
  }
#endif /* MCODE_COMMAND_MODES */

  bool start_uart_editor = true;

  if (!strcmp_P (aString, PSTR("help")))
  {
    /* HELP command */
    hw_uart_write_string_P(PSTR("Supported cmds:\r\n"));
#ifdef MCODE_CONSOLE_ENABLED
    hw_uart_write_string_P(PSTR("> color xxxx - set text color\r\n"));
    hw_uart_write_string_P(PSTR("> bg xxxx - set background color\r\n"));
    hw_uart_write_string_P(PSTR("> cls - Clear screen\r\n"));
#endif /* MCODE_CONSOLE_ENABLED */
#ifdef MCODE_TEST_IMAGES
    hw_uart_write_string_P(PSTR("> timg - Load test image\r\n"));
    hw_uart_write_string_P(PSTR("> tlimg - Load large test image\r\n"));
#endif /* MCODE_TEST_IMAGES */
#ifdef MCODE_CONSOLE_ENABLED
    cmd_engine_console_help();
#endif /* MCODE_CONSOLE_ENABLED */
#ifdef MCODE_PWM
    hw_uart_write_string_P(PSTR("> pwm <ind> <value> - Set PWM value\r\n"));
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
    hw_uart_write_string_P(PSTR("> led <IND> <1/0> - Turn ON/OFF the LEDs\r\n"));
#endif /* MCODE_LEDS */
#ifdef MCODE_LCD
    cmd_engine_lcd_help();
#endif /* MCODE_LCD */
#ifdef MCODE_SECURITY
    cmd_engine_ssl_help();
#endif /* MCODE_SECURITY */
#ifdef MCODE_COMMAND_MODES
    hw_uart_write_string_P(PSTR("> su [MODE(1|2|3)] - Set the command engine mode\r\n"));
    hw_uart_write_string_P(PSTR("> passwd - change the device password\r\n"));
#endif /* MCODE_COMMAND_MODES */
#ifdef MCODE_TWI
    cmd_engine_twi_help();
#endif /* MCODE_TWI */
#ifdef MCODE_RTC
    cmd_engine_rtc_help();
#endif /* MCODE_RTC */
    cmd_engine_system_help();
#ifdef MCODE_RTC
    cmd_engine_tv_help();
#endif /* MCODE_RTC */
#ifdef MCODE_CONSOLE_ENABLED
    cmd_engine_console_help();
#endif /* MCODE_CONSOLE_ENABLED */
  }
#ifdef MCODE_PWM
  else if (!strncmp_P(aString, PSTR("pwm "), 4)) {
    cmd_engine_pwm(&aString[4]);
  }
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
  else if (!strncmp_P(aString, PSTR("led "), 4)) {
    cmd_engine_set_led(&aString[4]);
  }
#endif /* MCODE_LEDS */
#ifdef MCODE_TEST_IMAGES
  else if (!strcmp_P(aString, PSTR("timg"))) {
    cmd_test_image();
  } else if (!strcmp_P(aString, PSTR("tlimg"))) {
    cmd_test_image_large();
  }
#endif /* MCODE_TEST_IMAGES */
#ifdef MCODE_SECURITY
  else if (cmd_engine_ssl_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_SECURITY */
#ifdef MCODE_COMMAND_MODES
  else if (!strncmp_P(aString, PSTR("su "), 3)) {
    cmd_engine_set_cmd_mode(&aString[3]);
  } else if (!strcmp_P(aString, PSTR("passwd"))) {
    cmd_engine_passwd();
  }
#endif /* MCODE_COMMAND_MODES */
#ifdef MCODE_TWI
  else if (cmd_engine_twi_read(aString)) {
    start_uart_editor = false;
  } else if (cmd_engine_twi_write(aString)) {
    start_uart_editor = false;
  }
#endif /* MCODE_TWI */
#ifdef MCODE_RTC
  else if (cmd_engine_rtc_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_RTC */
#ifdef MCODE_TV
  else if (cmd_engine_tv_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_TV */
#ifdef MCODE_LCD
  else if (cmd_engine_lcd_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_LCD */
#ifdef MCODE_CONSOLE_ENABLED
  else if (cmd_engine_console_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_CONSOLE_ENABLED */
  else if (cmd_engine_system_command(aString, &start_uart_editor)) {
  }
  else if (*aString) {
    /* got unrecognized non-empty command */
    hw_uart_write_string_P(PSTR("ENGINE: unrecognized command. Type 'help'.\r\n"));
  }

#ifdef MCODE_COMMAND_MODES
  if (TheCommandEngineState == CommandEngineStateCmd && start_uart_editor) {
    line_editor_uart_start();
  }
#else /* MCODE_COMMAND_MODES */
  if (start_uart_editor) {
    line_editor_uart_start();
  }
#endif /* MCODE_COMMAND_MODES */
}

#ifdef MCODE_PWM
void cmd_engine_pwm(const char *cmd)
{
  int index = -1;
  int value = -1;
  const char *const secondNumber = string_skip_whitespace(string_next_number(string_skip_whitespace(cmd), &index));
  if (secondNumber) {
    string_next_number(secondNumber, &value);
  }

  if (index < 0 || index > 2 || value < 0 || value > 255) {
    hw_uart_write_string_P(PSTR("Wrong args, format: pwm <index> <value>\r\n"));
    hw_uart_write_string_P(PSTR("Possible indexes: [0..2], values: [0..255]\r\n"));
    return;
  }

  pwm_set(index, value);
}
#endif /* MCODE_PWM */

#ifdef MCODE_LEDS
/**
 * Set-LED command handler
 * @param[in] aCommand - The command parameters
 * @note The command parameters should go in the following format: "X Y";
 * X may be either 0 or 1 (LED index) and Y may be either 0 or 1 (OFF or ON).
 */
void cmd_engine_set_led(const char *cmd)
{
  int index = -1;
  int value = -1;
  const char *const secondNumber = string_skip_whitespace(string_next_number(string_skip_whitespace(cmd), &index));
  if (secondNumber) {
    string_next_number(secondNumber, &value);
  }
  if (index >= 0) {
    if (value >= 0) {
      mcode_hw_leds_set(index, value);
    } else {
      const uint8_t on = mcode_hw_leds_get(index);
      hw_uart_write_string_P(PSTR("LED"));
      hw_uart_write_uint(index);
      hw_uart_write_string_P(PSTR(", value: "));
      hw_uart_write_uint(on);
      hw_uart_write_string_P(PSTR("\r\n"));
    }
  } else {
    hw_uart_write_string_P (PSTR("Wrong args, format: L I 0/1\r\n> I - the LED index [0..1]\r\n> 0/1 - turm OFF/ON\r\n"));
  }
}
#endif /* MCODE_LEDS */

#ifdef MCODE_COMMAND_MODES
typedef enum {
  ModesStateIdle,
  ModesStateEnterPass,
  ModesStateChangePassEnterCurrent,
  ModesStateChangePassEnterNew,
  ModesStateChangePassConfirmNew
} ModesState;

static uint8_t TheModesState;

void cmd_engine_set_mode(CmdMode mode)
{
  switch (mode) {
  case CmdModeRoot:
    /* super-user mode timeout: 5mins */
    TheSuperUserTimeout = 300000UL;
    /* fall through */
  case CmdModeNormal:
  case CmdModeUser:
    TheMode = mode;
    break;
  default:
    hw_uart_write_string_P(PSTR("Error: 'cmd_engine_set_mode' - wrong mode: 0x"));
    hw_uart_write_uint(mode);
    hw_uart_write_string_P(PSTR("\r\n"));
    break;
  }
}

CmdMode cmd_engine_get_mode(void)
{
  return (CmdMode)TheMode;
}

void cmd_engine_set_cmd_mode(const char *params)
{
  int value = 0;
  const char *next = string_next_number(string_skip_whitespace(params), &value);
  if (next && !*next && value > 0 && value < 4) {
    TheCommandEngineStateRequest = (CmdMode)(value - 1);
    hw_uart_write_string_P(PSTR("Enter password: "));
    TheCommandEngineState = CommandEngineStatePass;
    TheModesState = ModesStateEnterPass;
    line_editor_set_echo(false);
  } else {
    hw_uart_write_string_P(PSTR("Error: wrong arguments for 'su'.\r\nFormat: $ su [1/2/3]\r\n"));
  }
}

void cmd_engine_handle_args(const char *args)
{
  hw_uart_write_string_P(PSTR("Got args: '"));
  hw_uart_write_string(args);
  hw_uart_write_string_P(PSTR("'\r\n"));
  TheCommandEngineState = CommandEngineStateCmd;
  line_editor_uart_start();
}

void cmd_engine_handle_pass(const char *pass)
{
  const uint16_t n = strlen(pass);
  uint8_t hash[SHA256_DIGEST_LENGTH];
  uint8_t passHash[SHA256_DIGEST_LENGTH];
  SHA256((const uint8_t *)pass, n, passHash);
  if (ModesStateEnterPass == TheModesState ||
      ModesStateChangePassEnterCurrent == TheModesState) {
    persist_store_load(PersistStoreIdHash, hash, SHA256_DIGEST_LENGTH);

    if (!memcmp(hash, passHash, SHA256_DIGEST_LENGTH)) {
      cmd_engine_set_mode((CmdMode)TheCommandEngineStateRequest);
    } else {
      hw_uart_write_string_P(PSTR("Error: something wrong\r\n"));

      line_editor_set_echo(true);
      TheModesState = ModesStateIdle;
      TheCommandEngineState = CommandEngineStateCmd;
      line_editor_uart_start();
    }

    if (ModesStateEnterPass == TheModesState) {
      line_editor_set_echo(true);
      TheModesState = ModesStateIdle;
      TheCommandEngineState = CommandEngineStateCmd;
      line_editor_uart_start();
    } else if (ModesStateChangePassEnterCurrent == TheModesState) {
      hw_uart_write_string_P(PSTR("Enter new password: "));
      TheModesState = ModesStateChangePassEnterNew;
    }
  } else if (ModesStateChangePassEnterNew == TheModesState) {
    persist_store_save(PersistStoreIdNewHash, passHash, SHA256_DIGEST_LENGTH);
    hw_uart_write_string_P(PSTR("Confirm new password: "));
    TheModesState = ModesStateChangePassConfirmNew;
  } else if (ModesStateChangePassConfirmNew == TheModesState) {
    persist_store_load(PersistStoreIdNewHash, hash, SHA256_DIGEST_LENGTH);

    if (!memcmp(hash, passHash, SHA256_DIGEST_LENGTH)) {
      persist_store_save(PersistStoreIdHash, passHash, SHA256_DIGEST_LENGTH);
    } else {
      hw_uart_write_string_P(PSTR("Error: password missmatch\r\n"));
    }

    line_editor_set_echo(true);
    TheModesState = ModesStateIdle;
    TheCommandEngineState = CommandEngineStateCmd;
    line_editor_uart_start();
  }
}

void cmd_engine_mtick(void)
{
  if (TheSuperUserTimeout && !(--TheSuperUserTimeout)) {
    if (CmdModeRoot == TheMode) {
      cmd_engine_set_mode(CmdModeNormal);
    }
  }
}

void cmd_engine_passwd(void)
{
  hw_uart_write_string_P(PSTR("Enter current password: "));
  TheCommandEngineState = CommandEngineStatePass;
  TheModesState = ModesStateChangePassEnterCurrent;
  line_editor_set_echo(false);
}

#endif /* MCODE_COMMAND_MODES */
