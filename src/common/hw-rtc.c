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

#include "hw-rtc.h"

#include "mglobal.h"

const char *mtime_get_day_of_week_name(uint8_t dayOfWeek)
{
  switch (dayOfWeek) {
  case 1:
    return PSTR("MON");
  case 2:
    return PSTR("TUE");
  case 3:
    return PSTR("WED");
  case 4:
    return PSTR("THU");
  case 5:
    return PSTR("FRI");
  case 6:
    return PSTR("SAT");
  case 7:
    return PSTR("SUN");
  default:
    return PSTR("UNKWN");
  }
}

const char *mtime_get_month_name(uint8_t month)
{
  switch (month) {
  case 1:
    return PSTR("Jan");
  case 2:
    return PSTR("Feb");
  case 3:
    return PSTR("Mar");
  case 4:
    return PSTR("Apr");
  case 5:
    return PSTR("May");
  case 6:
    return PSTR("Jun");
  case 7:
    return PSTR("Jul");
  case 8:
    return PSTR("Aug");
  case 9:
    return PSTR("Sep");
  case 10:
    return PSTR("Oct");
  case 11:
    return PSTR("Nov");
  case 12:
    return PSTR("Dec");
  default:
    return PSTR("MONTH");
  }
}
