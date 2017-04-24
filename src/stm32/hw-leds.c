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

#include "hw-leds.h"

#include "mstring.h"

#include <stm32f10x.h>

#if !defined (STM32F10X_HD) && !defined (STM32F10X_MD)
#error "Unsupported device"
#endif /* !STM32F10X_HD && !STM32F10X_MD */

/*
 * Test code that manages 2 test LEDs connected to PB2, PB3
 */
void leds_init(void)
{
  /* GPIO configuration */
  /* Enable clocks */
#ifdef STM32F10X_HD
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
#elif defined (STM32F10X_MD)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
#endif /* STM32F10X_HD || STM32F10X_MD */

  /* Configure the pins */
#ifdef STM32F10X_HD
  GPIO_InitTypeDef GPIO_Config;
  GPIO_Config.GPIO_Pin =  GPIO_Pin_5;
  GPIO_Config.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Config.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_Config);
  GPIO_Config.GPIO_Pin =  GPIO_Pin_8;
  GPIO_Init(GPIOB, &GPIO_Config);
  GPIO_Config.GPIO_Pin =  GPIO_Pin_9;
  GPIO_Init(GPIOB, &GPIO_Config);
  GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
  GPIO_WriteBit(GPIOB, GPIO_Pin_8, Bit_RESET);
  GPIO_WriteBit(GPIOB, GPIO_Pin_9, Bit_RESET);
#elif defined (STM32F10X_MD)
  GPIO_InitTypeDef GPIO_Config;
  GPIO_Config.GPIO_Pin =  GPIO_Pin_13;
  GPIO_Config.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Config.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_Config);
  GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
#endif /* STM32F10X_HD || STM32F10X_MD */
}

void leds_deinit(void)
{
}

void leds_set(int index, int on)
{
  const BitAction value = on ? Bit_SET : Bit_RESET;
  switch (index) {
  case 1:
#ifdef STM32F10X_HD
    GPIO_WriteBit(GPIOB, GPIO_Pin_5, value);
#elif defined (STM32F10X_MD)
    GPIO_WriteBit(GPIOC, GPIO_Pin_13, value);
#endif /* STM32F10X_HD || STM32F10X_MD */
    break;
#ifdef STM32F10X_HD
  case 2:
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, value);
    break;
  case 3:
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, value);
    break;
#endif /* STM32F10X_HD */
  default:
    mprintstr("Undefined LED: ");
    mprint_uintd(index, 0);
    mprint(MStringNewLine);
    break;
  }
}

int leds_get(int index)
{
  int value = 0;
  switch (index) {
  case 1:
#ifdef STM32F10X_HD
    value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
#elif defined (STM32F10X_MD)
    value = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
#endif /* STM32F10X_HD || STM32F10X_MD */
    break;
#ifdef STM32F10X_HD
  case 2:
    value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);
    break;
  case 3:
    value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
    break;
#endif /* STM32F10X_HD */
  default:
    break;
  }
  return value;
}
