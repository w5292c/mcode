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

#include "main.h"
#include "hw-uart.h"
#include "hw-leds.h"
#include "scheduler.h"
#include "emu-common.h"
#include "cmd-engine.h"
#include "emu-common.h"
#include "line-editor-uart.h"

#include <stdio.h>
#include <signal.h>
#include <QApplication>

static void main_at_exit (void);
static void main_sigint_handler (int signo);
static void main_line_callback (const char *aString);

int main (int argc, char **argv)
{
  QApplication app(argc, argv);

  /* override the signal handler */
  if (SIG_ERR == signal(SIGINT, main_sigint_handler))
  {
    hw_uart_write_string_P (PSTR("Error: cannot override signal handler\r\n"));
    return -1;
  }
  atexit (main_at_exit);

  /* first, init the scheduler */
  mcode_scheduler_init ();
  /* now, UART can be initialized */
  hw_uart_init ();
  /* init LEDs */
  mcode_hw_leds_init ();
  /* init the line editor and the command engine */
  line_editor_uart_init ();
  cmd_engine_init ();

  /* now, enable the interrupts */
  sei ();

  /* Write some 'hello' text */
  hw_uart_write_string_P (PSTR("main: ready\r\nTest value: ["));
  hw_uart_write_uint (0x12afu);
  hw_uart_write_string_P (PSTR("]\r\n"));
  /* start the command engine, and switch to the scheduler, it never exits */
  cmd_engine_start ();
  mcode_scheduler_start ();

  cmd_engine_deinit ();
  line_editor_uart_deinit ();
  mcode_scheduler_deinit ();
  hw_uart_deinit ();
  pthread_exit (NULL);
  return 0;
}

static void main_at_exit (void)
{
  cmd_engine_deinit ();
}

void main_sigint_handler (int signo)
{
  if (SIGINT == signo)
  {
    hw_uart_write_string_P (PSTR("MAIN: got exit signal\r\n"));
    mcode_scheduler_stop ();
  }
}

void main_request_exit (void)
{
  hw_uart_write_string_P (PSTR("MAIN: exit request\r\n"));
  mcode_scheduler_stop ();
}

void mcode_main_start (void)
{
  QApplication::exec ();
}

void mcode_main_quit (void)
{
  QApplication::exit (0);
}
