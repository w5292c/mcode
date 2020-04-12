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

int main(int argc, char **argv)
{
  int res;

  res = pthread_create(&TheReadThread, NULL, sim_read_thread, NULL);
  if (-1 == res) exit(1);
  res = pthread_create(&TheWriteThread, NULL, sim_write_thread, NULL);
  if (-1 == res) exit(1);

  sleep(1);
  sim_send("RDY\r\n");
  sleep(1);
  sim_send("Call Ready\r\n");
  sleep(1);
  sim_send("SMS Ready\r\n");
  sleep(1);
  sim_send("+CPIN: READY\r\n");

  pthread_join(TheReadThread, NULL);
  pthread_join(TheWriteThread, NULL);

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
  if (!strcmp(cmd, "AT")) {
    printf("AT cmd, sending OK\n");
    sim_send("OK\r\n");
  } else if (!strcmp(cmd, "AT+CMGS=\"+70001112233\"")) {
    sim_send("> ");
  } else if (!strcmp(cmd, "hello\x1a")) {
    printf("Got SMS body, sending OK\n");
    sim_send("OK\r\n");
  } else {
    printf("Unknown cmd: [%s], sending ERROR\n", cmd);
    sim_send("ERROR\r\n");
  }
}
