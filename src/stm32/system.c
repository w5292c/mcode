/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2017 Alexander Chumakov
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

#include "system.h"

#include "mtick.h"
#include "hw-leds.h"
#include "scheduler.h"

#include <stm32f10x.h>

static void system_mtick(void);

void system_init(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Configure the power pin */
  GPIO_InitTypeDef pinConfig;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  /* Configure PB12 pin (npower) - Output, Open Dain */
  pinConfig.GPIO_Pin = GPIO_Pin_12;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOB, &pinConfig);
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);

  /* Sensing the power button */
  pinConfig.GPIO_Pin = GPIO_Pin_13;
  pinConfig.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &pinConfig);

  mtick_add(system_mtick);
}

void system_deinit(void)
{
}

void poweroff(void)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
}

void reboot(void)
{
  NVIC_SystemReset();
}

void system_mtick(void)
{
  static uint32_t timeout = 0;
  static uint32_t state = 0;
  static uint32_t storedValue = false;

  const uint32_t value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
  if (storedValue && !value) {
    storedValue = value;
    /* Power button press deticted */
    if (!state) {
      timeout = 1000;
    } else {
      state = 0;
      timeout = 0;
    }
  }
  if (!storedValue && value) {
    storedValue = value;
    /* Power button release deticted*/
    if (state) {
      mtick_sleep(1000);
      mcode_scheduler_stop();
    } else {
      timeout = 0;
    }
  }

  if (!timeout || --timeout) {
    return;
  }

  /* Power-off condition detected, wait for power button release */
  state = 1;
  leds_set(1, true);
}
