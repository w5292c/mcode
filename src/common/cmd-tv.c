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

#include "mtick.h"
#include "utils.h"
#include "hw-pwm.h"
#include "hw-leds.h"
#include "mglobal.h"
#include "mstring.h"
#include "hw-sound.h"
#include "scheduler.h"
#include "persistent-store.h"

#ifdef MCODE_PWM
#include <stdlib.h>
#endif /* MCODE_PWM */

typedef enum {
  CmdEngineTvStateOff = 0,
  CmdEngineTvStateOn,
} CmdEngineTvState;

static void cmd_engine_tv_tick(void);
static void cmd_engine_turn_tv_on(void);
static void cmd_engine_turn_tv_off(void);
static void cmd_engine_tv_update_value(void);
static void cmd_engine_tv_set_state(uint8_t state);
static void cmd_engine_handle_new_value(uint16_t value);
static bool cmd_engine_set_value(const char *args, bool *startCmd);
static bool cmd_engine_set_ititial_value(const char *args, bool *startCmd);

static uint8_t TheState;
static uint64_t TheNextUpdateTime;
static volatile bool TheExternalInterrupt;

void cmd_engine_tv_init(void)
{
  cmd_engine_tv_set_state(CmdEngineTvStateOff);
  scheduler_add(cmd_engine_tv_tick);
#ifdef __AVR__
  cmd_engine_tv_init_avr();
#endif /* __AVR__ */
}

void cmd_engine_tv_help(void)
{
  mprintstrln(PSTR("> tv - Show if TV is ON or OFF"));
  mprintstrln(PSTR("> tv-on - Turn the TV on"));
  mprintstrln(PSTR("> tv-off - Turn the TV off"));
  mprintstrln(PSTR("> value - Show persistent value"));
  mprintstrln(PSTR("> value-set <number> - Update persistent value to <number>"));
  mprintstrln(PSTR("> value-init - Show the current initial value"));
  mprintstrln(PSTR("> value-init-set <value> - Set the initial value to <value>"));
}

bool cmd_engine_tv_command(const char *args, bool *startCmd)
{
  if (!strcmp_P(args, PSTR("value"))) {
    mprintstr(PSTR("Persistent value: "));
    mprint_uintd(persist_store_get_value(), 0);
    mprint(MStringNewLine);
    return true;
  } else if (!strncmp_P(args, PSTR("value-set "), 10)) {
    return cmd_engine_set_value(args + 10, startCmd);
  } else if (!strcmp_P(args, PSTR("tv-on"))) {
    cmd_engine_tv_emulate_ext_request(true);
    return true;
  } else if (!strcmp_P(args, PSTR("tv-off"))) {
    cmd_engine_tv_emulate_ext_request(false);
    return true;
  } else if (!strcmp_P(args, PSTR("value-init"))) {
    mprintstr(PSTR("Initial value: "));
    mprint_uintd(persist_store_get_initial_value(), 0);
    mprint(MStringNewLine);
    return true;
  } else if (!strncmp_P(args, PSTR("value-init-set "), 15)) {
    return cmd_engine_set_ititial_value(args + 15, startCmd);
  } else if (!strcmp_P(args, PSTR("tv"))) {
    if (CmdEngineTvStateOn == TheState) {
      mprintstrln(PSTR("TV is ON"));
    } else {
      mprintstrln(PSTR("TV is OFF"));
    }
    return true;
  }

  return false;
}

bool cmd_engine_set_value(const char *args, bool *startCmd)
{
  /* Get the 'number' argument */
  uint16_t number = -1;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &number);
  if (args || number == -1) {
    /* Wrong 'number' argument */
    merror(MStringWrongArgument);
    return true;
  }

#ifdef MCODE_COMMAND_MODES
  if (CmdModeRoot != cmd_engine_get_mode()) {
    merror(MStringWrongMode);
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

  persist_store_set_value(number);
  cmd_engine_handle_new_value(number);
  return true;
}

void cmd_engine_tv_new_day(void)
{
  const uint16_t initialValue = persist_store_get_initial_value();
  if (persist_store_get_value() == initialValue) {
    /* The current value is already initial, no need to update */
    mprintstrln(PSTR("New day: values match"));
    return;
  }

  persist_store_set_value(initialValue);
  cmd_engine_handle_new_value(initialValue);
  mprintstrln(PSTR("New day: updated to initial value"));

  if (CmdEngineTvStateOn == TheState) {
    TheNextUpdateTime = mtick_count() + 60LU*1000;
    cmd_engine_tv_update_value();
  }
}

void cmd_engine_tv_tick(void)
{
  if (TheExternalInterrupt) {
    TheExternalInterrupt = false;

    if (cmd_engine_tv_ext_request()) {
      cmd_engine_tv_set_state(CmdEngineTvStateOn);
    } else {
      cmd_engine_tv_set_state(CmdEngineTvStateOff);
    }
  }

  if (CmdEngineTvStateOn == TheState) {
#ifdef MCODE_PWM
    static uint16_t delay = 0;
    if (!delay--) {
      delay = ((uint32_t)rand()*300)/RAND_MAX;
      const uint8_t value = ((uint32_t)rand())*255/RAND_MAX;
      pwm_set(2, value);
    }
#endif /* MCODE_PWM */

    const uint64_t currentTime = mtick_count();
    if (TheNextUpdateTime < currentTime) {
      /* Expire after another minute */
      TheNextUpdateTime = currentTime + 60LU*1000;
      cmd_engine_tv_update_value();
    }
  }
}

bool cmd_engine_set_ititial_value(const char *args, bool *startCmd)
{
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    /* Too few arguments */
    merror(MStringWrongArgument);
    return true;
  }
  /* Get the 'value' argument */
  uint16_t value = -1;
  args = string_next_number(args, &value);
  if (args || value == -1) {
    /* Wrong 'number' argument */
    merror(MStringWrongArgument);
    return true;
  }

#ifdef MCODE_COMMAND_MODES
  if (CmdModeRoot != cmd_engine_get_mode()) {
    merror(MStringWrongMode);
    return true;
  }
#endif /* MCODE_COMMAND_MODES */

  persist_store_set_initial_value(value);
  return true;
}

void cmd_engine_tv_update_value(void)
{
  const uint16_t value = persist_store_get_value();
  if (!value) {
    /* No more time for today */
    cmd_engine_tv_set_state(CmdEngineTvStateOff);
    return;
  }

  persist_store_set_value(value - 1);
  cmd_engine_handle_new_value(value - 1);
}

void cmd_engine_tv_set_state(uint8_t state)
{
  if (TheState == state) {
    /* Already in the requested state */
    return;
  }

  if (CmdEngineTvStateOn == state) {
    cmd_engine_turn_tv_on();
    /* Requested ON state, check we we still have enough time */
    const uint16_t value = persist_store_get_value();
    if (!value) {
      /* No more time */
#if 0
      mprintstrln(PSTR("Error: no more time"));
#endif
      cmd_engine_turn_tv_off();
      return;
    }

    persist_store_set_value(value - 1);
    cmd_engine_handle_new_value(value - 1);
    TheNextUpdateTime = mtick_count() + 60LU*1000;
    cmd_engine_turn_tv_on();
  } else {
    cmd_engine_turn_tv_off();
  }

  TheState = state;
}

void cmd_engine_handle_new_value(uint16_t value)
{
#ifdef MCODE_SOUND
  switch (value) {
  case 0:
    sound_play_note(0x58, 2000);
    sound_play_note(0xff, 200);
  case 1:
    sound_play_note(0x56, 500);
    sound_play_note(0xff, 200);
  case 2:
    sound_play_note(0x54, 500);
    sound_play_note(0xff, 200);
  case 3:
    sound_play_note(0x52, 500);
    sound_play_note(0xff, 200);
  case 4:
    sound_play_note(0x50, 500);
    sound_play_note(0xff, 200);
    break;
  default:
    break;
  }
#endif /* MCODE_SOUND */
}

void cmd_engine_turn_tv_on(void)
{
#ifdef MCODE_LEDS
  leds_set(0, true);
#endif /* MCODE_LEDS */

  cmd_engine_tv_turn(true);
}

void cmd_engine_turn_tv_off(void)
{
#ifdef MCODE_LEDS
  leds_set(0, false);
#endif /* MCODE_LEDS */

#ifdef MCODE_PWM
  pwm_set(2, 0);
#endif /* MCODE_PWM */

  cmd_engine_tv_turn(false);
}

void cmd_engine_tv_ext_req_changed_interrupt(void)
{
  TheExternalInterrupt = true;
}
