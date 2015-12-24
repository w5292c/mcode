/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Alexander Chumakov
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

#include "sha.h"
#include "mtick.h"
#include "utils.h"
#include "hw-uart.h"
#include "mglobal.h"
#include "strings.h"
#include "line-editor-uart.h"
#include "persistent-store.h"

#include <string.h>

#ifdef MCODE_COMMAND_MODES
typedef enum {
  CommandEngineStateCmd,
  CommandEngineStatePass,
} CommandEngineState;

typedef enum {
  ModesStateIdle,
  ModesStateEnterPass,
  ModesStateChangePassEnterCurrent,
  ModesStateChangePassEnterNew,
  ModesStateChangePassConfirmNew
} ModesState;

static uint8_t TheMode;
static uint8_t TheModesState;
static uint32_t TheSuperUserTimeout;
static uint8_t TheCommandEngineState = CommandEngineStateCmd;
static uint8_t TheCommandEngineStateRequest;
static void cmd_engine_mtick(void);
static void cmd_engine_passwd(void);
static void cmd_engine_handle_pass(const char *pass);
static void cmd_engine_set_cmd_mode(const char *params);
#endif /* MCODE_COMMAND_MODES */

static void cmd_engine_sha256(const char *aParams);

void cmd_engine_ssl_init(void)
{
#ifdef MCODE_COMMAND_MODES
  TheMode = CmdModeNormal;
  TheSuperUserTimeout = 0;
  mtick_add(cmd_engine_mtick);
#endif /* MCODE_COMMAND_MODES */
}

void cmd_engine_ssl_help(void)
{
  hw_uart_write_string_P(PSTR("> sha256 <DATA> - calculate SHA256 hash\r\n"));
}

bool cmd_engine_ssl_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("sha256 "), 7)) {
    cmd_engine_sha256(command + 7);
    return true;
  }
#ifdef MCODE_COMMAND_MODES
  else if (!strncmp_P(command, PSTR("su "), 3)) {
    cmd_engine_set_cmd_mode(&command[3]);
    *startCmd = false;
    return true;
  } else if (!strcmp_P(command, PSTR("passwd"))) {
    cmd_engine_passwd();
    *startCmd = false;
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

  return false;
}

void cmd_engine_sha256(const char *aParams)
{
  const uint16_t n = strlen(aParams);
  hw_uart_write_string_P(PSTR("Calculating sha256 hash, string: '"));
  hw_uart_write_string(aParams);
  hw_uart_write_string_P(PSTR("', length: 0x"));
  hw_uart_write_uint(n);
  hw_uart_write_string_P(PSTR("\r\n"));

  uint8_t byteResultHash[SHA256_DIGEST_LENGTH];
  SHA256((const unsigned char *)aParams, n, byteResultHash);

  uint8_t i;
  uint8_t *ptr = byteResultHash;
  for (i = 0; i < SHA256_DIGEST_LENGTH; i += 2, ptr += 2) {
    uint16_t data = ((*ptr) << 8) | (*(ptr + 1) << 0);
    hw_uart_write_uint16(data, false);
  }
  hw_uart_write_string_P(PSTR("\r\n"));
}

#ifdef MCODE_COMMAND_MODES
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

void cmd_engine_set_cmd_mode(const char *args)
{
  uint16_t value = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &value);
  if (!args && value > 0 && value < 4) {
    TheCommandEngineStateRequest = (CmdMode)(value - 1);
    hw_uart_write_string_P(PSTR("Enter password: "));
    TheCommandEngineState = CommandEngineStatePass;
    TheModesState = ModesStateEnterPass;
    line_editor_set_echo(false);
    line_editor_uart_set_callback(cmd_engine_handle_pass);
  } else {
    merror(MStringWrongArgument);
  }
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
      cmd_engine_start();
    }

    if (ModesStateEnterPass == TheModesState) {
      line_editor_set_echo(true);
      TheModesState = ModesStateIdle;
      TheCommandEngineState = CommandEngineStateCmd;
      cmd_engine_start();
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
    cmd_engine_start();
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
  line_editor_uart_set_callback(cmd_engine_handle_pass);
}
#endif /* MCODE_COMMAND_MODES */
