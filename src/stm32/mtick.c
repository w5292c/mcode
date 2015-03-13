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

#include "scheduler.h"

#include <stdbool.h>
#include <stm32f10x.h>

#ifndef MCODE_MTICKS_COUNT
#define MCODE_MTICKS_COUNT (10)
#endif /* MCODE_MTICKS_COUNT */

static volatile uint64_t TheMSecCounter = 0;
static volatile bool TheSceduledFlag = false;
static mcode_tick TheTickCallbacks[MCODE_MTICKS_COUNT];

static void mcode_mtick_scheduler_tick(void);

void mtick_init(void)
{
  /* first, prepare the TheTickCallbacks */
  int i;
  for (i = 0; i < MCODE_MTICKS_COUNT; ++i) {
    TheTickCallbacks[i] = 0;
  }

  /* Enable the TIM6 global Interrupt */
  NVIC_InitTypeDef nvicConfig;
  nvicConfig.NVIC_IRQChannel = TIM6_IRQn;
  nvicConfig.NVIC_IRQChannelPreemptionPriority = 0;
  nvicConfig.NVIC_IRQChannelSubPriority = 1;
  nvicConfig.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicConfig);

  /* Define PCLK1 clock: HCLK/1 = 72MHz */
  RCC_PCLK1Config(RCC_HCLK_Div1);
  /* Enable the TIM6 clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  /* Configure the TIM6 timer to generate a 1ms interrupt */
  TIM_TimeBaseInitTypeDef tim6Config;
  tim6Config.TIM_Prescaler = 3;
  tim6Config.TIM_CounterMode = TIM_CounterMode_Up;
  tim6Config.TIM_Period = 17999;
  tim6Config.TIM_ClockDivision = 0;
  tim6Config.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM6, &tim6Config);
  /* Enable the 'update' interrupt */
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

  /* Start counting */
  TIM_Cmd(TIM6, ENABLE);

  mcode_scheduler_add(mcode_mtick_scheduler_tick);
}

void mtick_deinit(void)
{
}

void mtick_add(mcode_tick tick)
{
  int i;
  for (i = 0; i < MCODE_MTICKS_COUNT; ++i) {
    if (!TheTickCallbacks[i]) {
      TheTickCallbacks[i] = tick;
      break;
    }
  }
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

void TIM6_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

    /* update the state */
    TheSceduledFlag = true;
    ++TheMSecCounter;
  }
}

void mcode_mtick_scheduler_tick(void)
{
  TIM_ITConfig(TIM6, TIM_IT_Update, DISABLE);
  if (TheSceduledFlag) {
    TheSceduledFlag = false;
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    int i;
    mcode_tick tick;
    for (i = 0; i < MCODE_MTICKS_COUNT; ++i) {
      tick = TheTickCallbacks[i];
      if (!tick) {
        break;
      }

      /* Invoke the tick */
      (*tick)();
    }
  }
  else {
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  }
}
