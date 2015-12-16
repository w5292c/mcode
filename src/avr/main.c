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

#include "pwm.h"
#include "mtick.h"
#include "hw-lcd.h"
#include "hw-i2c.h"
#include "hw-leds.h"
#include "hw-uart.h"
#include "console.h"
#include "scheduler.h"
#include "cmd-engine.h"
#include "line-editor-uart.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

int main (void)
{
  /* first, init the scheduler */
  mcode_scheduler_init();
  mtick_init();
#ifdef MCODE_I2C
  i2c_init();
#endif /* MCODE_I2C */
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
  mcode_hw_leds_init();
#endif /* MCODE_LEDS */
#ifdef MCODE_PWM
  pwm_init();
#endif /* MCODE_PWM */
  /* init the line editor and the command engine */
  line_editor_uart_init();
  cmd_engine_init();

  /* now, enable the interrupts */
  sei();

  /* Write some 'hello' text */
  hw_uart_write_string_P(PSTR("\r\nmain: ready\r\nTest value: ["));
  hw_uart_write_uintd(12345067, true);
  hw_uart_write_string_P(PSTR("]\r\n"));
  cmd_engine_start();

  /* start the scheduler, it never exits */
  mcode_scheduler_start();

  /* Clean-up */
#ifdef MCODE_LCD
  lcd_deinit();
#endif /* MCODE_LCD */

#ifdef MCODE_I2C
  i2c_deinit();
#endif /* MCODE_I2C */

  mtick_deinit();
  return 0;
}
