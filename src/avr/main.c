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

#include "mcode-config.h"

#include "mtick.h"
#include "hw-lcd.h"
#include "hw-twi.h"
#include "hw-rtc.h"
#include "hw-pwm.h"
#include "mstring.h"
#include "hw-leds.h"
#include "hw-uart.h"
#include "console.h"
#include "hw-sound.h"
#include "scheduler.h"
#include "cmd-engine.h"
#include "line-editor-uart.h"

#include <avr/interrupt.h>

int main (void)
{
  /* first, init the scheduler */
  mcode_scheduler_init();
  mtick_init();

#ifdef MCODE_TWI
  twi_init();
#endif /* MCODE_TWI */

#ifdef MCODE_RTC
  mtime_init();
  rtc_alarm_init();
#endif /* MCODE_RTC */

  /* now, UART can be initialized */
  hw_uart_init();
#ifdef MCODE_LCD
  lcd_init(320, 480);
#endif /* MCODE_LCD */
#ifdef MCODE_CONSOLE
  console_init();
#endif /* MCODE_CONSOLE */
#ifdef MCODE_LEDS
  /* init LEDs */
  leds_init();
#endif /* MCODE_LEDS */
#ifdef MCODE_PWM
  pwm_init();
#endif /* MCODE_PWM */
  /* init the line editor and the command engine */
  line_editor_uart_init();
  cmd_engine_init();

#ifdef MCODE_SOUND
  sound_init();
#endif /* MCODE_SOUND */

#ifdef MCODE_TV
  cmd_engine_tv_init();
#endif /* MCODE_TV */

#ifdef MCODE_SECURITY
  cmd_engine_ssl_init();
#endif /* MCODE_SECURITY */

  /* now, enable the interrupts */
  sei();

#ifdef MCODE_LCD
  /* Now, as interrupts are enabled, we can reset LCD */
  lcd_reset();
#endif /* MCODE_LCD */

  /* Write some 'hello' text */
  mprintstr(PSTR("\r\nmain: ready\r\nTest value: ["));
  mprint_uintd(1234007, 0);
  mprintstrln(PSTR("]\r\n"));
  cmd_engine_start();

  /* start the scheduler, it never exits */
  mcode_scheduler_start();

#ifdef MCODE_SOUND
  sound_deinit();
#endif /* MCODE_SOUND */

  /* Clean-up */
#ifdef MCODE_LCD
  lcd_deinit();
#endif /* MCODE_LCD */

#ifdef MCODE_RTC
  rtc_alarm_deinit();
  mtime_deinit();
#endif /* MCODE_RTC */

#ifdef MCODE_TWI
  twi_deinit();
#endif /* MCODE_TWI */

  mtick_deinit();
  return 0;
}
