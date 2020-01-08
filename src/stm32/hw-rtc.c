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

#include "hw-rtc.h"

#include "stm32f10x_bkp.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rtc.h"

void mtime_init(void)
{
}

void mtime_deinit(void)
{
}

void rtc_alarm_init(void)
{
}

void rtc_alarm_deinit(void)
{
}

void rtc_first_time_init(void)
{
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable LSE */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {
  }

  /* Select LSE as RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();

  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

void mtime_get_time(mtime_time_ready callback)
{
  MTime time;
  const uint32_t counter = RTC_GetCounter();
  rtc_get_time(counter, &time);
  (*callback)(true, &time);
}

void mtime_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback)
{
  MDate date;
  MTime time;
  uint32_t counter = RTC_GetCounter();
  /* Keep the current date, update only time */
  rtc_get_date(counter, &date);

  time.seconds = seconds;
  time.minutes = minutes;
  time.hours = hours;

  counter = rtc_to_mtime(&date, &time);
  RTC_SetCounter(counter);

  (*callback)(true);
}

void mtime_get_date(mtime_date_ready callback)
{
  MDate date;
  const uint32_t counter = RTC_GetCounter();
  rtc_get_date(counter, &date);
  (*callback)(true, &date);
}

void mtime_set_date(int16_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, mcode_done callback)
{
  MDate date;
  MTime time;
  uint32_t counter = RTC_GetCounter();
  /* Keep the current time, update only date */
  rtc_get_time(counter, &time);

  date.dayOfWeek = dayOfWeek;
  date.day = day;
  date.month = month;
  date.year = year;

  counter = rtc_to_mtime(&date, &time);
  RTC_SetCounter(counter);

  (*callback)(true);
}

void mtime_set_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback)
{
  /* Not supported for now */
  (*callback)(false);
}

void mtime_set_new_day_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds, mcode_done callback)
{
  (void)hours;
  (void)minutes;
  (void)seconds;

  /* Not supported */
  callback(false);
}
