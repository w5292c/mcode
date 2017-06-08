/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Alexander Chumakov
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


#include "gsm-engine.h"

#include <stm32f10x.h>

void hw_gsm_init(void)
{
  /* Configure GSM power pin */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  GPIO_InitTypeDef pinConfig;
  /* Configure the GSM-POWER pin, initially, power it off */
  pinConfig.GPIO_Pin =  GPIO_Pin_3;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_OD;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &pinConfig);

  /* Turn GSM power ON on default */
  hw_gsm_power(true);
}

void hw_gsm_deinit(void)
{
}

void hw_gsm_power(bool on)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_3, on ? Bit_RESET : Bit_SET);
}
