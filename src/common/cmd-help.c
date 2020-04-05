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

#include "cmd-iface.h"

#include "mglobal.h"
#include "mparser.h"
#include "mstring.h"
#include "cmd-engine.h"

#include <stddef.h>

static bool cmd_help(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd);
static bool cmd_old_help(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd);

static const char TheBaseHelp[] PROGMEM = ("help");
static const char TheBaseOldHelp[] PROGMEM = ("help-old");
static const char TheHelpMessage[] PROGMEM = ("Show help for all commands");
static const char TheOldHelpMessage[] PROGMEM = ("Show help for old commands");

CMD_ENTRY(TheBaseHelp, TheHelp, TheHelpMessage, &cmd_help, NULL, 0);
CMD_ENTRY(TheBaseOldHelp, TheOldHelp, TheOldHelpMessage, &cmd_old_help, NULL, 0);

bool cmd_help(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  const TCmdData *iter = &__start_command_section;
  const TCmdData *const end = &__stop_command_section;

  if (!args_len) {
    /* List all supported commands */
    bool first_cmd = true;
    mprintstr(PSTR("Commands: ["));
    for (; iter < end; ++ iter) {
      if (!first_cmd) {
        mprintstr(PSTR(", "));
      }
      const char *const base = pgm_read_ptr_near(&iter->base);
      mprintstr(base);
      first_cmd = false;
    }
    mprintstrln(PSTR("]"));
  } else {
    /* Show help for a specific command */
    for (; iter < end; ++ iter) {
      const char *const base = pgm_read_ptr_near(&iter->base);
      if (!mparser_strcmp_P(args, args_len, base)) {
        mprintstr(PSTR("> "));
        mprintbytes_R(args, args_len);
        mprintstr(PSTR(" - "));
        const char *const help = pgm_read_ptr_near(&iter->help);
        mprintstr(help);
        break;
      }
    }
    mprint(MStringNewLine);
  }

  return true;
}

bool cmd_old_help(const TCmdData *data, const char *args, size_t args_len, bool *start_cmd)
{
  mprintstrln(PSTR("Supported cmds:"));
#ifdef MCODE_TEST_IMAGES
  cmd_engine_images_help();
#endif /* MCODE_TEST_IMAGES */
#ifdef MCODE_PWM
  cmd_engine_pwm_help();
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
  cmd_engine_led_help();
#endif /* MCODE_LEDS */
#ifdef MCODE_LCD
  cmd_engine_lcd_help();
#endif /* MCODE_LCD */
#ifdef MCODE_SECURITY
  cmd_engine_ssl_help();
#endif /* MCODE_SECURITY */
#ifdef MCODE_COMMAND_MODES
  mprintstrln(PSTR("> su [MODE(1|2|3)] - Set the command engine mode"));
  mprintstrln(PSTR("> passwd - change the device password"));
#endif /* MCODE_COMMAND_MODES */
#ifdef MCODE_TWI
  cmd_engine_twi_help();
#endif /* MCODE_TWI */
#ifdef MCODE_RTC
  cmd_engine_rtc_help();
#endif /* MCODE_RTC */
  cmd_engine_system_help();
#ifdef MCODE_TV
  cmd_engine_tv_help();
#endif /* MCODE_TV */
#ifdef MCODE_CONSOLE_ENABLED
  cmd_engine_console_help();
#endif /* MCODE_CONSOLE_ENABLED */
#ifdef MCODE_SOUND
  cmd_engine_sound_help();
#endif /* MCODE_SOUND */
#ifdef MCODE_GSM
  cmd_engine_gsm_help();
#endif /* MCODE_GSM */
#ifdef MCODE_SWITCH_ENGINE
  cmd_engine_switch_help();
#endif /* MCODE_SWITCH_ENGINE */
#ifdef MCODE_PROG
  cmd_engine_prog_help();
#endif /* MCODE_PROG */

  return true;
}
