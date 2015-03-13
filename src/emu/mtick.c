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

#include "mtick.h"

#include <glib.h>

static uint64_t TheMSecCounter = 0;
static guint TheTimeoutSourceId = 0;

static gboolean mtick_timeout_handler(gpointer data);

void mtick_init(void)
{
  if (!TheTimeoutSourceId) {
    TheTimeoutSourceId = g_timeout_add(1, mtick_timeout_handler, NULL);
  }
}

void mtick_deinit(void)
{
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

gboolean mtick_timeout_handler(gpointer data)
{
  ++TheMSecCounter;
  return TRUE;
}
