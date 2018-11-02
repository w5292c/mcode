/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2018 Alexander Chumakov
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

#include "switch-engine.h"

#include "mtick.h"
#include "scheduler.h"

#include <stm32f10x.h>

/**
 * HW configuration:
 * - L1: PA8: Prot 0;
 * - L2: PB4: Power 0;
 * - L3: PB1: Prot 1;
 * - L4: PB0: Power 1;
 *
 * States sequence:
 *     REQ(ON, SECONDS)           2 sec.    SECONDS secs.             2 sec.
 * Off ===============> Disarming =====> ON ============> Turning OFF =====> OFF
 *                      | Protection OFF | Power ON       | Power OFF        | Protection ON
 */

#define MCODE_SWITCH_ENGINE_ITEMS_COUNT (2)
#define MCODE_SWITCH_PROTECTION_TIMEOUT (2000)

typedef enum {
  ESwitchEngineStateOff = 0,
  ESwitchEngineStateTurningOn,
  ESwitchEngineStateOn,
  ESwitchEngineStateTurningOff,
} MSwitchEngineState;

typedef struct {
  MSwitchEngineState state;
  uint32_t seconds;
  uint64_t requestTimeStamp;
} MSwitchEngineItem;

static MSwitchEngineItem TheItems[MCODE_SWITCH_ENGINE_ITEMS_COUNT] = {{0}};

static void switch_engine_tick(void);
static void switch_engine_turn_prot(int index, bool on);
static void switch_engine_turn_power(int index, bool on);

void switch_engine_init(void)
{
  /* Configure GSM power pin */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

  GPIO_InitTypeDef pinConfig;
  /* Configure the Protection-0 pin, initially, power it off (turn-ON) */
  pinConfig.GPIO_Pin =  GPIO_Pin_8;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &pinConfig);
  GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);

  /* Configure the POWER-0 pin, initially, power it off */
  pinConfig.GPIO_Pin =  GPIO_Pin_4;
  GPIO_Init(GPIOB, &pinConfig);
  GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);

  /* Configure the Protection-1 pin, initially, power it off (turn-ON) */
  pinConfig.GPIO_Pin =  GPIO_Pin_0;
  GPIO_Init(GPIOB, &pinConfig);
  GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);

  /* Configure the POWER-1 pin, initially, power it off */
  pinConfig.GPIO_Pin =  GPIO_Pin_1;
  GPIO_Init(GPIOB, &pinConfig);
  GPIO_WriteBit(GPIOB, GPIO_Pin_2, Bit_RESET);

  scheduler_add(switch_engine_tick);
}

void switch_engine_deinit(void)
{
}

void switch_engine_turn_off(uint32_t index)
{
  if (index >= MCODE_SWITCH_ENGINE_ITEMS_COUNT) {
    /* Wrong index */
    return;
  }

  MSwitchEngineItem *const item = TheItems + index;
  switch (item->state) {
  case ESwitchEngineStateOn:
    switch_engine_turn_power(index, false);

    item->requestTimeStamp = mtick_count();
    item->state = ESwitchEngineStateTurningOff;
    break;
  case ESwitchEngineStateTurningOn:
    switch_engine_turn_prot(index, false);

    item->state = ESwitchEngineStateOff;
    break;
  default:
  case ESwitchEngineStateOff:
  case ESwitchEngineStateTurningOff:
    break;
  }
}

void switch_engine_turn_on(uint32_t index, uint32_t seconds)
{
  if (index >= MCODE_SWITCH_ENGINE_ITEMS_COUNT) {
    /* Wrong index */
    return;
  }

  MSwitchEngineItem *const item = TheItems + index;
  switch (item->state) {
  case ESwitchEngineStateOff:
    switch_engine_turn_prot(index, true);
    /* No break */
  case ESwitchEngineStateTurningOff:
    item->state = ESwitchEngineStateTurningOn;
    /* No break */
  case ESwitchEngineStateOn:
  case ESwitchEngineStateTurningOn:
    item->seconds = seconds;
    item->requestTimeStamp = mtick_count();
    break;
  default:
    break;
  }
}

void switch_engine_tick(void)
{
  /** @todo Make 'tick' less heavy, moving 'target' calculation to state transition */
  int i;
  for (i = 0; i < MCODE_SWITCH_ENGINE_ITEMS_COUNT; ++i) {
    MSwitchEngineItem *const item = TheItems + i;
    if (ESwitchEngineStateOn == item->state) {
      const uint64_t target = item->requestTimeStamp + item->seconds*MCODE_MSECS_IN_A_SECOND;
      if (mtick_count() > target) {
        /* Timeout for ON state, start turning OFF */
        switch_engine_turn_power(i, false);

        item->requestTimeStamp = mtick_count();
        item->state = ESwitchEngineStateTurningOff;
      }
    } else if (ESwitchEngineStateTurningOn == item->state) {
      const uint64_t target = item->requestTimeStamp + MCODE_SWITCH_PROTECTION_TIMEOUT;
      if (mtick_count() > target) {
        /* Protection has been turned OFF, ready to turn power ON */
        item->requestTimeStamp = mtick_count();
        item->state = ESwitchEngineStateOn;

        switch_engine_turn_power(i, true);
      }
    } else if (ESwitchEngineStateTurningOff == item->state) {
      const uint64_t target = item->requestTimeStamp + MCODE_SWITCH_PROTECTION_TIMEOUT;
      if (mtick_count() > target) {
        /* Power should be turned OFF now, ready to turn protection ON */
        switch_engine_turn_prot(i, false);

        item->state = ESwitchEngineStateOff;
      }
    }
  }
}

void switch_engine_turn_prot(int index, bool on)
{
  switch (index) {
  case 0:
    GPIO_WriteBit(GPIOA, GPIO_Pin_8, on ? Bit_SET : Bit_RESET);
    break;
  case 1:
    GPIO_WriteBit(GPIOB, GPIO_Pin_1, on ? Bit_SET : Bit_RESET);
    break;
  default:
    break;
  }
}

void switch_engine_turn_power(int index, bool on)
{
  switch (index) {
  case 0:
    GPIO_WriteBit(GPIOB, GPIO_Pin_4, on ? Bit_SET : Bit_RESET);
    break;
  case 1:
    GPIO_WriteBit(GPIOB, GPIO_Pin_0, on ? Bit_SET : Bit_RESET);
    break;
  default:
    break;
  }
}
