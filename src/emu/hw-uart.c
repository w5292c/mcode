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

#include "hw-uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>

static int running_request = 0;
static pthread_t TheKeyEventThread = 0;
static struct termios TheStoredTermIos;

/** The UART emulation thread */
static void *emu_hw_uart_thread (void *threadid);

void hw_uart_init(void)
{
  if (!TheKeyEventThread) {
    /* get the original termios information */
    tcgetattr(STDIN_FILENO, &TheStoredTermIos);

    long t = 0;
    running_request = 1;
    const int failed = pthread_create(&TheKeyEventThread, NULL, emu_hw_uart_thread, (void *)t);
    if (failed) {
      running_request = 0;
      hw_uart_write_string("Error: cannot create key-event thread, code: ");
      hw_uart_write_uint(failed);
      hw_uart_write_string("\r\n");
      exit (-1);
    }
  }
}

void hw_uart_deinit(void)
{
  if (TheKeyEventThread) {
    int failed = pthread_cancel (TheKeyEventThread);
    if (failed) {
      hw_uart_write_string("Error: cannot cancel thread, code: ");
      hw_uart_write_uint(failed);
      hw_uart_write_string("\r\n");
      exit (-1);
    }
    void *status;
    running_request = 0;
    failed = pthread_join(TheKeyEventThread, &status);
    if (failed) {
      hw_uart_write_string("Error: cannot join key-event thread, code: ");
      hw_uart_write_uint(failed);
      hw_uart_write_string("\r\n");
      exit (-1);
    }

    /* restore the original termios attributes */
    tcsetattr( STDIN_FILENO, TCSANOW, &TheStoredTermIos);
    TheKeyEventThread = 0;
  }
}

static hw_uart_char_event TheCallback = NULL;
void hw_uart_set_callback(hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void uart_write_char(char ch)
{
  putchar(ch);
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
      if (escIndex) {
        (*TheCallback) (escChar);
        escIndex = 0;
      } else {
        (*TheCallback) (ch);
      }
    }
  }

  /* terminating thread */
  hw_uart_write_string("emu_hw_uart_thread, finishing: ");
  hw_uart_write_uint(1234);
  hw_uart_write_string("\r\n");
  pthread_exit(NULL);
  return NULL;
}
