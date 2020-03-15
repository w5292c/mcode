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

#include "main.h"
#include "mtick.h"
#include "hw-lcd.h"
#include "mtimer.h"
#include "mstring.h"
#include "console.h"
#include "hw-uart.h"
#include "hw-leds.h"
#include "scheduler.h"
#include "cmd-engine.h"
#include "line-editor-uart.h"

#include <QDebug>
#include <stdio.h>
#include <signal.h>
#include <QStringList>
#include <QApplication>

static uint16_t TheWidth = 240;
static uint16_t TheHeight = 320;

static void main_at_exit (void);
static void main_sigint_handler (int signo);
static void main_line_callback (const char *aString);

int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  const QStringList &arguments = QCoreApplication::arguments();
  // Parsing arguments
  const uint sizeIndex = arguments.indexOf("-s");
  if (sizeIndex >= 0 && argc >= sizeIndex + 2) {
    const QString &sizeString = arguments[sizeIndex + 1];
    const QStringList &sizes = sizeString.split("x");
    if (2 == sizes.size()) {
      bool ok1 = false;
      bool ok2 = false;
      const uint proposedWidth = sizes[0].toUInt(&ok1);
      const uint proposedHeight = sizes[1].toUInt(&ok2);
      if (ok1 && ok2 && proposedWidth && proposedHeight) {
        TheWidth = proposedWidth;
        TheHeight = proposedHeight;
      }
    }
  }
  qDebug() << "MAIN: LCD resolution: (" << TheWidth << "X" << TheHeight << ")";

  /* override the signal handler */
  if (SIG_ERR == signal(SIGINT, main_sigint_handler)) {
    mprintstrln(PSTR("Error: cannot override signal handler"));
    return -1;
  }
  atexit(main_at_exit);

  /* first, init the scheduler */
  scheduler_init();
  mtick_init();
  mtimer_init();
  /* now, UART can be initialized */
  hw_uart_init();
  lcd_init(TheWidth, TheHeight);
  /* init LEDs */
  leds_init();
  /* init the line editor and the command engine */
  line_editor_uart_init();
  cmd_engine_init();
  console_init();

  /* Write some 'hello' text */
  mprintstr(PSTR("main: ready\r\nTest value: ["));
  mprint_uintd(0x12afu, 0);
  mprintstrln(PSTR("]"));
  /* start the command engine, and switch to the scheduler, it never exits */
  cmd_engine_start();

  /* Start the QT4 event loop in the main thread */
  const int res = app.exec();

  cmd_engine_deinit();
  line_editor_uart_deinit();
  lcd_deinit();
  mtimer_deinit();
  mtick_deinit();
  scheduler_deinit();
  hw_uart_deinit();
  pthread_exit(NULL);
  return res;
}

static void main_at_exit(void)
{
  cmd_engine_deinit();
}

void main_sigint_handler(int signo)
{
  if (SIGINT == signo) {
    mprintstrln(PSTR("MAIN: got exit signal"));
    main_request_exit();
  }
}

void main_request_exit(void)
{
  mprintstrln(PSTR("MAIN: exit request"));
  QApplication::exit(0);
}

uint16_t main_base_width(void)
{
  return TheWidth;
}

uint16_t main_base_height(void)
{
  return TheHeight;
}
