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

#include "cmd-iface.h"
#include "cmd-engine.h"

#include "mglobal.h"
#include "mparser.h"
#include "mstatus.h"
#include "mstring.h"
#include "switch-engine.h"

CMD_IMPL("switch", TheSwitch, "Turn <on/off> switch <num> for <seconds>", cmd_switch, NULL, 0);

bool cmd_switch(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  bool on;
  TokenType type;
  uint32_t index;
  uint32_t value;
  const char *token;

  /* Check the 1st argument, it should be either 'on' or 'off' */
  type = next_token(&args, &args_len, &token, &value);
  if (TokenString != type) {
    mcode_errno_set(EArgument);
    return true;
  }

  /* Check the value of the 1st argument */
  if (!mparser_strcmp_P(token, value, PSTR("on"))) {
    on = true;
  } else if (mparser_strcmp_P(token, value, PSTR("off"))) {
    mcode_errno_set(EArgument);
    return true;
  }

  /* Parse the 2nd parameter - the switch index, skip the whitespeces first */
  do {
    type = next_token(&args, &args_len, &token, &index);
  } while (TokenWhitespace == type);

  if (TokenInt != type) {
    mcode_errno_set(EArgument);
    return true;
  }

  if (on) {
    /* Skip whitespeces */
    do {
      type = next_token(&args, &args_len, &token, &value);
    } while (TokenWhitespace == type);

    /* For the 'on' case, we need to have the 3rd parameter - amount of time for on-state */
    if (TokenInt != type) {
      mcode_errno_set(EArgument);
      return true;
    }

    switch_engine_turn_on(index, value);
  } else {
    switch_engine_turn_off(index);
  }
}
