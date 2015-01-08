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

#include "hw-uart.h"

#include <stm32f10x.h>

/*
 * Test code that manages 2 test LEDs connected to PB2, PB3
 */
void mcode_hw_leds_init (void)
{
  /* GPIO configuration */
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Configure the pins */
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
}

void mcode_hw_leds_deinit (void)
{
}

void mcode_hw_leds_set (int index, int on)
{
  const BitAction value = on ? Bit_SET : Bit_RESET;
  switch (index) {
  case 1:
    GPIO_WriteBit(GPIOB, GPIO_Pin_5, value);
    break;
  case 2:
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, value);
    break;
  case 3:
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, value);
    break;
  default:
    hw_uart_write_string("Undefined LED: ");
    hw_uart_write_uint(index);
    hw_uart_write_string("\r\n");
    break;
  }
}

int mcode_hw_leds_get (int index)
{
  int value = 0;
  switch (index) {
  case 1:
    value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
    break;
  case 2:
    value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);
    break;
  case 3:
    value = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
    break;
  default:
    break;
  }
  return value;
}
