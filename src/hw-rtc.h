/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Alexander Chumakov
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

#ifndef MCODE_RTC_H
#define MCODE_RTC_H

#include "mcode-config.h"

#include "mglobal.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCODE_RTC

typedef struct {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
} MTime;

typedef struct {
  uint8_t dayOfWeek;
  uint8_t day;
  uint8_t month;
  int16_t year;
} MDate;

typedef void (*mtime_time_ready)(bool success, const MTime *time);
typedef void (*mtime_date_ready)(bool success, const MDate *time);

void mtime_init(void);
void mtime_deinit(void);

void mtime_get_time(mtime_time_ready callback);
void mtime_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback);

void mtime_get_date(mtime_date_ready callback);
void mtime_set_date(uint8_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, mcode_done callback);

const char *mtime_get_month_name(uint8_t month);
const char *mtime_get_day_of_week_name(uint8_t dayOfWeek);

#endif /* MCODE_RTC */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_RTC_H */
