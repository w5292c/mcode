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

#include "hw-uart.h"

#include "mglobal.h"
#include "mstring.h"
#include "scheduler.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <errno.h>

static bool TheQuitRequest = false;
static char TheBuffer[128] = {0};
static size_t TheBufferWrIndex = 0;
static size_t TheBufferRdIndex = 0;
static int running_request = 0;
static pthread_t TheKeyEventThread = 0;
static struct termios TheStoredTermIos;

#ifdef MCODE_UART2
static int TheInPipe = 0;
static int TheOutPipe = 0;
static bool TheOutPipeOpened = false;
static char TheUart2OutBuffer[256] = {0};
static size_t TheUart2OutBufferRdIndex = 0;
static size_t TheUart2OutBufferWrIndex = 0;
static pthread_t TheUart2ReadThreadId = 0;
static pthread_t TheUart2WriteThreadId = 0;

static void *emu_hw_uart2_read_thread(void *arg);
static void *emu_hw_uart2_write_thread(void *args);
#endif /* MCODE_UART2 */

static void emu_hw_uart_tick(void);
/** The UART emulation thread */
static void *emu_hw_uart_thread(void *threadid);

void hw_uart_init(void)
{
  int res;

  if (!TheKeyEventThread) {
    /* get the original termios information */
    tcgetattr(STDIN_FILENO, &TheStoredTermIos);

    long t = 0;
    running_request = 1;
    res = pthread_create(&TheKeyEventThread, NULL, emu_hw_uart_thread, (void *)t);
    if (res) {
      running_request = 0;
      mprintstr(PSTR("Error: cannot create key-event thread, code: "));
      mprint_uintd(res, 0);
      mprint(MStringNewLine);
      exit(-1);
    }

    scheduler_add(emu_hw_uart_tick);
  }

#ifdef MCODE_UART2
  if (!TheUart2ReadThreadId) {
    res = pthread_create(&TheUart2ReadThreadId, NULL, emu_hw_uart2_read_thread, NULL);
    if (res) {
      exit(-1);
    }
  }
  if (!TheUart2WriteThreadId) {
    res = pthread_create(&TheUart2WriteThreadId, NULL, emu_hw_uart2_write_thread, NULL);
    if (res) {
      exit(-1);
    }
  }
#endif /* MCODE_UART2 */
}

void hw_uart_deinit(void)
{
  TheQuitRequest = true;
  if (TheKeyEventThread) {
    int failed = pthread_cancel (TheKeyEventThread);
    if (failed) {
      mprintstr(PSTR("Error: cannot cancel thread, code: "));
      mprint_uintd(failed, 0);
      mprint(MStringNewLine);
      exit(-1);
    }
    running_request = 0;
    failed = pthread_join(TheKeyEventThread, NULL);
    if (failed) {
      mprintstr(PSTR("Error: cannot join key-event thread, code: "));
      mprint_uintd(failed, 0);
      mprint(MStringNewLine);
      exit(-1);
    }

    /* restore the original termios attributes */
    tcsetattr( STDIN_FILENO, TCSANOW, &TheStoredTermIos);
    TheKeyEventThread = 0;
  }

#ifdef MCODE_UART2
  if (TheUart2ReadThreadId) {
    pthread_join(TheUart2ReadThreadId, NULL);
  }
  if (TheUart2WriteThreadId) {
    if (!TheOutPipeOpened) {
      pthread_cancel(TheUart2WriteThreadId);
    }
    pthread_join(TheUart2WriteThreadId, NULL);
  }
#endif /* MCODE_UART2 */
}

static hw_uart_char_event TheCallback = NULL;
void hw_uart_set_callback(hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void uart_write_char(char ch)
{
  putchar(ch);
  fflush(stdout);
}

void *emu_hw_uart_thread(void *threadid)
{
  int escIndex = 0;
  unsigned int escChar = 0;

  /* upate termios attributes, so, we receive getchar on each character entered, no echo */
  static struct termios newt;
  newt = TheStoredTermIos;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  while (running_request) {
    const unsigned int ch = getchar ();
    if (escIndex) {
      escChar = escChar | (ch << (8 * escIndex));
      ++escIndex;
    }
    if (27 == ch) {
      escChar = ch;
      escIndex = 1;
    }

    if (TheCallback && (escIndex == 3 || (!escIndex))) {
      if (TheBufferWrIndex == sizeof (TheBuffer)) {
        TheBufferWrIndex = 0;
      }
      if (escIndex) {
        TheBuffer[TheBufferWrIndex] = escChar;
        escIndex = 0;
      } else {
        TheBuffer[TheBufferWrIndex] = ch;
      }

      ++TheBufferWrIndex;
    }
  }

  /* terminating thread */
  mprintstr(PSTR("emu_hw_uart_thread, finishing: "));
  mprint_uintd(1234, 0);
  mprint(MStringNewLine);
  pthread_exit(NULL);
  return NULL;
}

void emu_hw_uart_tick(void)
{
  if (TheBufferWrIndex != TheBufferRdIndex) {
    if (TheBufferRdIndex == sizeof (TheBuffer)) {
      TheBufferRdIndex = 0;
    }
    const char ch = TheBuffer[TheBufferRdIndex];
    TheBuffer[TheBufferRdIndex] = 0;
    ++TheBufferRdIndex;
    TheCallback(ch);
  }

#ifdef MCODE_UART2
  /* Handle the input data */
  uart2_report_new_sample();
#endif /* MCODE_UART2 */
}

#ifdef MCODE_UART2
void uart2_write_char(char ch)
{
  if (TheUart2OutBufferWrIndex >= sizeof (TheUart2OutBuffer)) {
    TheUart2OutBufferWrIndex = 0;
  }
  TheUart2OutBuffer[TheUart2OutBufferWrIndex++] = ch;
}

void *emu_hw_uart2_read_thread(void *arg)
{
  int res;

  res = mkfifo("/var/tmp/sim-to-mcode", S_IRUSR | S_IWUSR);
  if (-1 == res && EEXIST != errno) exit(1);

  TheInPipe = open("/var/tmp/sim-to-mcode", O_RDONLY | O_NONBLOCK);
  if (-1 == TheInPipe) {
    exit(1);
  }

  char buf;
  while (!TheQuitRequest) {
    res = read(TheInPipe, &buf, 1);
    if (1 == res) {
      uart2_handle_new_sample(buf);
    }
  }

  close(TheInPipe);
  return NULL;
}

void *emu_hw_uart2_write_thread(void *args)
{
  int res;

  res = mkfifo("/var/tmp/mcode-to-sim", S_IRUSR | S_IWUSR);
  if (-1 == res && EEXIST != errno) exit(1);

  TheOutPipe = open("/var/tmp/mcode-to-sim", O_WRONLY);
  if (-1 == TheOutPipe) {
    exit(1);
  }
  TheOutPipeOpened = true;
  while (!TheQuitRequest) {
    char ch;
    if (TheUart2OutBufferWrIndex != TheUart2OutBufferRdIndex) {
      if (TheUart2OutBufferRdIndex >= sizeof (TheUart2OutBuffer)) {
        TheUart2OutBufferWrIndex = 0;
      }
      ch = TheUart2OutBuffer[TheUart2OutBufferRdIndex++];
      write(TheOutPipe, &ch, 1);
    }
  }

  close(TheOutPipe);
  return NULL;
}
#endif /* MCODE_UART2 */
