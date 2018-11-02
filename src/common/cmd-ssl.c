/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2018 Alexander Chumakov
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
#include "mstring.h"
#include "scheduler.h"
#include "line-editor-uart.h"
#include "persistent-store.h"

#include <string.h>

#ifdef MCODE_COMMAND_MODES
static uint8_t TheMode;
static uint8_t *ThePointer;
static uint8_t TheSslSchedulerId;
static uint32_t TheSuperUserTimeout;
static void cmd_engine_mtick(void);
static void cmd_engine_passwd(void);
static void cmd_engine_on_pass(const char *string);
static void cmd_engine_set_cmd_mode(const char *params, bool *startCmd);
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
  mprintstrln(PSTR("> sha256 <DATA> - calculate SHA256 hash"));
}

bool cmd_engine_ssl_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("sha256 "), 7)) {
    cmd_engine_sha256(command + 7);
    return true;
  }
#ifdef MCODE_COMMAND_MODES
  else if (!strncmp_P(command, PSTR("su "), 3)) {
    cmd_engine_set_cmd_mode(command + 3, startCmd);
    return true;
  } else if (!strcmp_P(command, PSTR("passwd"))) {
    cmd_engine_passwd();
    line_editor_set_echo(true);
    cmd_engine_start();
    *startCmd = false;
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

  return false;
}

void cmd_engine_sha256(const char *aParams)
{
  const uint16_t n = strlen(aParams);
  mprintstr(PSTR("Calculating sha256 hash, string: '"));
  mprintstr_R(aParams);
  mprintstr(PSTR("', length: "));
  mprint_uintd(n, 0);
  mprint(MStringNewLine);

  uint8_t byteResultHash[SHA256_DIGEST_LENGTH];
  SHA256((const unsigned char *)aParams, n, byteResultHash);

  uint8_t i;
  uint8_t *ptr = byteResultHash;
  for (i = 0; i < SHA256_DIGEST_LENGTH; i += 2, ptr += 2) {
    uint16_t data = ((*ptr) << 8) | (*(ptr + 1) << 0);
    mprint_uint16(data, false);
  }
  mprint(MStringNewLine);
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
    break;
  }
}

CmdMode cmd_engine_get_mode(void)
{
  return (CmdMode)TheMode;
}

void cmd_engine_set_cmd_mode(const char *args, bool *startCmd)
{
  uint16_t value = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &value);
  if (args || !value || value > 3) {
    merror(MStringWrongArgument);
    return;
  }

  /* Convert 'value' to 'CmdMode' */
  --value;
  /* For the 'root' mode we need to check password */
  if (CmdModeRoot != value) {
    cmd_engine_set_mode((CmdMode)value);
    return;
  }
  if (TheSslSchedulerId) {
    merror(MStringInternalError);
    return;
  }

  uint8_t hash[SHA256_DIGEST_LENGTH];
  uint8_t storedHash[SHA256_DIGEST_LENGTH];
  ThePointer = hash;

  /* Enter the password */
  mprint(MStringEnterPass);
  line_editor_reset();
  line_editor_set_echo(false);
  line_editor_uart_set_callback(cmd_engine_on_pass);
  scheduler_start(&TheSslSchedulerId);
  line_editor_set_echo(true);

  /* Check the entered password */
  persist_store_load(PersistStoreIdHash, storedHash, SHA256_DIGEST_LENGTH);
  if (!memcmp(hash, storedHash, SHA256_DIGEST_LENGTH)) {
    cmd_engine_set_mode((CmdMode)value);
  } else {
    merror(MStringInternalError);
  }

  /* We need to revert line editor to the engine, so, start engine */
  *startCmd = false;
  cmd_engine_start();
}

void cmd_engine_on_pass(const char *string)
{
  const uint8_t length = strlen(string);
  SHA256((const uint8_t *)string, length, ThePointer);
  scheduler_stop(TheSslSchedulerId);
  TheSslSchedulerId = 0;
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
  if (TheSslSchedulerId) {
    merror(MStringInternalError);
    return;
  }

  /* First, we need to check the current password */
  uint8_t hash[SHA256_DIGEST_LENGTH];
  uint8_t tempHash[SHA256_DIGEST_LENGTH];
  ThePointer = hash;

  /* Enter the password */
  mprint(MStringEnterPass);
  line_editor_reset();
  line_editor_set_echo(false);
  line_editor_uart_set_callback(cmd_engine_on_pass);
  scheduler_start(&TheSslSchedulerId);

  /* Check the entered password */
  persist_store_load(PersistStoreIdHash, tempHash, SHA256_DIGEST_LENGTH);
  if (memcmp(hash, tempHash, SHA256_DIGEST_LENGTH)) {
    merror(MStringInternalError);
    return;
  }

  /* Now, enter the new password */
  mprintstr(PSTR("Enter new password: "));
  line_editor_reset();
  scheduler_start(&TheSslSchedulerId);

  ThePointer = tempHash;
  mprintstr(PSTR("Confirm new password: "));
  line_editor_reset();
  scheduler_start(&TheSslSchedulerId);

  if (memcmp(hash, tempHash, SHA256_DIGEST_LENGTH)) {
    /* Passwords do not match */
    merror(MStringInternalError);
    return;
  }

  persist_store_save(PersistStoreIdHash, hash, SHA256_DIGEST_LENGTH);
}
#endif /* MCODE_COMMAND_MODES */
