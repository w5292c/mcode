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

#include "mtick.h"

#include "mstring.h"

#include <stdlib.h>
#include <pthread.h>

static uint64_t TheMSecCounter = 0;
static pthread_t TheTimerThreadId = 0;
static bool TheMTickRunRequest = false;

static void *mtick_thread(void *args);

void mtick_init(void)
{
  int res;
  if (!TheTimerThreadId) {
    TheMTickRunRequest = true;
    res = pthread_create(&TheTimerThreadId, NULL, mtick_thread, NULL);
    if (res) {
      mprintstrln(PSTR("Error: failed creating the mtick thread"));
      exit(1);
    }
  }
}

void mtick_deinit(void)
{
  TheMTickRunRequest = false;
  pthread_join(TheTimerThreadId, NULL);
}

void mtick_add(mcode_tick tick)
{
}

void mtick_sleep(uint32_t mticks)
{
  const uint64_t target = TheMSecCounter + mticks + 1;
  while (TheMSecCounter < target);
}

uint64_t mtick_count(void)
{
  return TheMSecCounter;
}

void *mtick_thread(void *args)
{
  int res;
  struct timespec ts = {0};

  /* Get the current time and calculate the next timer expiration time,
     aligned to millisecond grid */
  res = clock_gettime(CLOCK_TAI, &ts);
  if (res) {
    mprintstrln(PSTR("Error: failed getting the system time"));
    exit(1);
  }
  ts.tv_nsec = (1 + ts.tv_nsec/1000000)*1000000;
  if (ts.tv_nsec >= 1000000000) {
    ts.tv_nsec = 0;
    ++ts.tv_sec;
  }

  while (TheMTickRunRequest) {
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &ts, NULL);
    ++TheMSecCounter;

    /* Update the expiration time to 1 millisecond in the future */
    ts.tv_nsec += 1000000;
    if (ts.tv_nsec >= 1000000000) {
      ++ts.tv_sec;
      ts.tv_nsec = 0;
    }
  }

  return NULL;
}
