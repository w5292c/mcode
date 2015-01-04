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

#include "scheduler.h"

#include <stm32f10x.h>

int main (void)
{
  /* GPIO configuration */
  GPIO_InitTypeDef GPIO_Config;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_Config.GPIO_Pin =  GPIO_Pin_5;
  GPIO_Config.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Config.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_Config);
  GPIO_Config.GPIO_Pin =  GPIO_Pin_8;
  GPIO_Init(GPIOB, &GPIO_Config);
  GPIO_Config.GPIO_Pin =  GPIO_Pin_9;
  GPIO_Init(GPIOB, &GPIO_Config);
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);

  /* first, init the scheduler */
  mcode_scheduler_init ();

#if 0
  mcode_scheduler_start ();
#else
  /* Implement something visible, to be removed later, as soon as implemented properly */
  int s = 0;
  for (;;) {
    switch(s) {
    case 0:
      GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);
      GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
      GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);
      break;
    case 1:
      GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
      GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_SET);
      GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);
      break;
    case 2:
      GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
      GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
      GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_SET);
      break;
    default:
      break;
    }

    for (unsigned int i=0;i<0xfffffU;i++);
    if (++s > 2) s = 0;
  }
#endif

  return 0;
}
