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

#define MCODE_INITIAL_YEAR (2001)
#define MCODE_DAYS_IN_A_WEEK (7)
#define MCODE_DAYS_IN_A_YEAR (365)
#define MCODE_DAYS_IN_4_YEARS (1461)
#define MCODE_SECONDS_IN_A_DAY (86400)

static int rtc_days_in_month(uint32_t month, bool leapYear);

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

void rtc_get_time(uint32_t mtime, MTime *time)
{
  uint32_t seconds = mtime % MCODE_SECONDS_IN_A_DAY;
  time->hours = seconds/3600;
  seconds -= time->hours * 3600;
  time->minutes = seconds / 60;
  time->seconds = seconds % 60;
}

void rtc_get_date(uint32_t mtime, MDate *date)
{
  /* Leave out time info, keep the number of days size initial date */
  mtime /= MCODE_SECONDS_IN_A_DAY;

  /* Calculate the week-day */
  date->dayOfWeek = (mtime % MCODE_DAYS_IN_A_WEEK) + 1;

  /*  */
  const uint32_t qyears = mtime / MCODE_DAYS_IN_4_YEARS;
  /* 'mtime' now is the number of days since 4-year period */
  mtime -= qyears * MCODE_DAYS_IN_4_YEARS;

  /* 'year1' will be 0..3 */
  uint32_t year1 = mtime / MCODE_DAYS_IN_A_YEAR;
  if (year1 > 3) {
    /* Correction for 31-Dec of leap years */
    year1 = 3;
  }
  const bool leapYear = (3 == year1);
  /* Now, 'mtime' is the number of days since 1-Jan of the calculated year */
  mtime -= year1 * MCODE_DAYS_IN_A_YEAR;

  /* Now, we know the year */
  date->year = MCODE_INITIAL_YEAR + 4*qyears + year1;

  /* Calculate the month */
  uint32_t month;
  for (month = 1; month <= 12; ++month) {
    const uint32_t days = rtc_days_in_month(month, leapYear);
    if (mtime > days) {
      mtime -= days;
    } else {
      date->month = month;
      break;
    }
  }

  date->day = mtime + 1;
}

uint32_t rtc_to_mtime(const MDate *date, const MTime *time)
{
  if (date->year < MCODE_INITIAL_YEAR || date->year > 2136) {
    /* No support for years before 'MCODE_INITIAL_YEAR' and after 2136 for now */
    return 0;
  }

  /* Time part */
  uint32_t mtime = time->seconds + time->minutes * 60 + time->hours * 3600;
  /* Adding date-of-month part */
  mtime +=  (date->day - 1) * MCODE_SECONDS_IN_A_DAY;

  uint32_t year = date->year - MCODE_INITIAL_YEAR;
  uint32_t qyear = year / 4;
  year -= qyear * 4;
  const bool leapYear = (year == 3);

  mtime += qyear * (MCODE_DAYS_IN_4_YEARS*MCODE_SECONDS_IN_A_DAY);
  mtime += year * (MCODE_DAYS_IN_A_YEAR*MCODE_SECONDS_IN_A_DAY);

  /* Finally, add the month info */
  for (int i = 1; i < date->month; ++i) {
    mtime += MCODE_SECONDS_IN_A_DAY*rtc_days_in_month(i, leapYear);
  }

  return mtime;
}

int rtc_days_in_month(uint32_t month, bool leapYear)
{
  switch (month) {
  case 1:
    return 31;
  case 2:
    return leapYear ? 29 : 28;
  case 3:
    return 31;
  case 4:
    return 30;
  case 5:
    return 31;
  case 6:
    return 30;
  case 7:
    return 31;
  case 8:
    return 31;
  case 9:
    return 30;
  case 10:
    return 31;
  case 11:
    return 30;
  case 12:
    return 31;
  default:
    return 0;
  }
}
