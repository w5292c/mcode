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

#include "mcode-config.h"

#include "mtick.h"
#include "hw-uart.h"
#include "mstring.h"
#include "scheduler.h"
#include "cmd-engine.h"
#include "line-editor-uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <errno.h>

static int TheInPipe = 0;
static int TheOutPipe = 0;

static pthread_t TheReadThread = 0;
static pthread_t TheWriteThread = 0;

static char TheInBuffer[256] = {0};
static char TheOutBuffer[1024] = {0};
static size_t TheInBufferWrIndex = 0;
static size_t TheOutBufferRdIndex = 0;
static size_t TheOutBufferWrIndex = 0;

static void *sim_read_thread(void *args);
static void *sim_write_thread(void *args);

static void sim_send(const char *rsp);
static void sim_handle_command(const char *cmd);

static void sim_dump(const char *str);

int main(int argc, char **argv)
{
  int res;

  /* first, init the scheduler */
  scheduler_init();
  mtick_init();
  /* now, UART can be initialized */
  hw_uart_init();
  /* init the line editor and the command engine */
  line_editor_uart_init();
  cmd_engine_init();

  /* start the command engine */
  cmd_engine_start();

  res = pthread_create(&TheReadThread, NULL, sim_read_thread, NULL);
  if (-1 == res) exit(1);
  res = pthread_create(&TheWriteThread, NULL, sim_write_thread, NULL);
  if (-1 == res) exit(1);

  /*
  READY event
  Full-functionality event
  PIN-READY event
  Call-READY event
  SMS-READY event
  */
  sleep(1);
  sim_send("RDY\r\n");
  sleep(1);
  sim_send("+CFUN: 1\r\n");
  sleep(1);
  sim_send("+CPIN: READY\r\n");
  sleep(1);
  sim_send("Call Ready\r\n");
  sleep(1);
  sim_send("SMS Ready\r\n");

  pthread_join(TheReadThread, NULL);
  pthread_join(TheWriteThread, NULL);

  mtick_deinit();
  scheduler_deinit();

  return 0;
}

void *sim_read_thread(void *args)
{
  int res;
  char ch;

  res = mkfifo("/var/tmp/mcode-to-sim", S_IRUSR | S_IWUSR);
  if (-1 == res && EEXIST != errno) exit(1);

  TheInPipe = open("/var/tmp/mcode-to-sim", O_RDONLY | O_NONBLOCK);
  if (-1 == TheInPipe) exit(1);

  while (true) {
    res = read(TheInPipe, &ch, 1);
    if (1 == res) {
      if ('\r' != ch) {
        TheInBuffer[TheInBufferWrIndex] = ch;
        ++TheInBufferWrIndex;
        continue;
      }
      TheInBuffer[TheInBufferWrIndex] = 0;
      sim_handle_command(TheInBuffer);
      TheInBufferWrIndex = 0;
    } else if (-1 == res && EAGAIN != errno) {
      fprintf(stderr, "Error: %d, %d\n", res, errno);
    }

    usleep(10000);
  }

  close(TheInPipe);

  return 0;
}

void *sim_write_thread(void *args)
{
  int res;

  res = mkfifo("/var/tmp/sim-to-mcode", S_IRUSR | S_IWUSR);
  if (-1 == res && EEXIST != errno) exit(1);

  TheOutPipe = open("/var/tmp/sim-to-mcode", O_WRONLY | O_NONBLOCK);

  char ch;
  while (true) {
    if (TheOutBufferWrIndex != TheOutBufferRdIndex) {
      if (TheOutBufferRdIndex == sizeof (TheOutBuffer)) {
        TheOutBufferRdIndex = 0;
      }

      ch = TheOutBuffer[TheOutBufferRdIndex];
      TheOutBuffer[TheOutBufferRdIndex] = 0;
      ++TheOutBufferRdIndex;
      write(TheOutPipe, &ch, 1);
    }

    usleep(1000);
  }

  close(TheOutPipe);
  return NULL;
}

void sim_send(const char *rsp)
{
  char ch;

  fprintf(stdout, "<<< \"");
  sim_dump(rsp);
  fprintf(stdout, "\"\n");
  while ((ch = *rsp++)) {
    if (TheOutBufferWrIndex == sizeof (TheOutBuffer)) {
      TheOutBufferWrIndex = 0;
    }
    TheOutBuffer[TheOutBufferWrIndex] = ch;
    ++TheOutBufferWrIndex;
  }
}

void sim_handle_command(const char *cmd)
{
  fprintf(stdout, ">>> \"");
  sim_dump(cmd);
  fprintf(stdout, "\"\n");
  if (!strcasecmp(cmd, "AT")) {
    sim_send("OK\r\n");
  } else if (!strcasecmp(cmd, "AT+CMGS=\"002B00370030003000300031003100310032003200330033\"")) {
    sim_send("> ");
  } else if (!strcmp(cmd, "00680065006C006C006F\x1a")) {
    sim_send("+CMGS: 2\r\n");
    usleep(10000);
    sim_send("OK\r\n");
  } else if (!strcasecmp(cmd, "AT+CMGR=1")) {
    sim_send("+CMGR: \"REC READ\",\"002B00390038003800370035003300310030003100320033\",\"\",\"20/01/08,10:27:04+12\"\r\n");
    usleep(10000);
    sim_send("005400650073007400200053004D0053000A004C0069006E006500200032003B000A002B0063006D00670072003D002200680065006C006C006F0022\r\n");
  } else {
    sim_send("ERROR\r\n");
  }
}

void sim_dump(const char *str)
{
  char ch;
  while ((ch = *str++)) {
    if (ch >= 32) {
      fprintf(stdout, "%c", ch);
    } else {
      fprintf(stdout, "[0x%02X]", (unsigned int)ch);
    }
  }
}
