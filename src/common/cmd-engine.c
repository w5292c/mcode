/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2020 Alexander Chumakov
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

#include "mglobal.h"
#include "mstring.h"
#include "cmd-iface.h"
#include "line-editor-uart.h"

#include <string.h>

static void cmd_engine_show_help(void);
static void cmd_engine_on_cmd_ready(const char *aString);

void cmd_engine_init(void)
{
  line_editor_uart_init();

#ifdef MCODE_GSM
  cmd_engine_gsm_init();
#endif /* MCODE_GSM */
}

void cmd_engine_deinit(void)
{
#ifdef MCODE_GSM
  cmd_engine_gsm_deinit();
#endif /* MCODE_GSM */

  line_editor_uart_deinit();
}

void cmd_engine_start(void)
{
  line_editor_uart_set_callback(cmd_engine_on_cmd_ready);
  line_editor_uart_start();
}

void cmd_engine_on_cmd_ready(const char *aString)
{
  bool start_uart_editor = true;

  if (!strcmp_P (aString, PSTR("help"))) {
    cmd_engine_show_help();
  }
#ifdef MCODE_PWM
  else if (cmd_engine_pwm_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
  else if (cmd_engine_led_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_LEDS */
#ifdef MCODE_TEST_IMAGES
  else if (cmd_engine_images_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_TEST_IMAGES */
#ifdef MCODE_SECURITY
  else if (cmd_engine_ssl_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_SECURITY */
#ifdef MCODE_TWI
  else if (cmd_engine_twi_command(aString, &start_uart_editor)) {
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
#ifdef MCODE_SOUND
  else if (cmd_engine_sound_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_SOUND */
#ifdef MCODE_GSM
  else if (cmd_engine_gsm_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_GSM */
#ifdef MCODE_SWITCH_ENGINE
  else if (cmd_engine_led_command(aString, &start_uart_editor)) {
  }
#endif /* MCODE_SWITCH_ENGINE */
#ifdef MCODE_PROG
  else if (cmd_engine_prog_exec(aString, &start_uart_editor)) {
  }
#endif /* MCODE_PROG */
  else if (cmd_engine_system_command(aString, &start_uart_editor)) {
  }
  else if (*aString) {
    /* got unrecognized non-empty command */
    mprintstrln(PSTR("ENGINE: unrecognized command. Type 'help'."));
  }

  if (start_uart_editor) {
    line_editor_uart_start();
  }
}

void cmd_engine_show_help(void)
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

  /* New style for commands/help support, using 'command_section' section */
  const char *help;
  const TCmdData *iter = &__start_command_section;
  const TCmdData *const end = &__stop_command_section;
  for (; iter < end; ++ iter) {
    help = pgm_read_ptr_near(&iter->help);
    mprintstr(help);
    mprint(MStringNewLine);
  }
}
