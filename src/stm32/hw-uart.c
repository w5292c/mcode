/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2017 Alexander Chumakov
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

#include "hw-uart.h"

#include "scheduler.h"

#include <stm32f10x.h>

static hw_uart_char_event TheCallback = NULL;
#ifdef MCODE_UART2
static hw_uart_char_event TheUart2Callback = NULL;
#endif /* MCODE_UART2 */

static void hw_uart_tick (void);

void hw_uart_init (void)
{
#if defined (STM32F10X_HD) || defined (STM32F10X_MD)
  /* Init clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

  /* Init USART pins */
  /* USART1_RX (in): PA10: IN_FLOATING */
  GPIO_InitTypeDef pinConfig;
  pinConfig.GPIO_Pin = GPIO_Pin_10;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  pinConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &pinConfig);
  /* USART1_TX (OUT): PA9: Alt_PP */
  pinConfig.GPIO_Pin = GPIO_Pin_9;
  pinConfig.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &pinConfig);

  /* Init USART device */
  USART_InitTypeDef initData;
  initData.USART_BaudRate = 115200;
  initData.USART_WordLength = USART_WordLength_8b;
  initData.USART_StopBits = USART_StopBits_2;
  initData.USART_Parity = USART_Parity_No;
  initData.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  initData.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART1, &initData);

  /* Start the device */
  USART_Cmd(USART1, ENABLE);

#ifdef MCODE_UART2
    /* Init USART2 clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

  /* Init USART2 pins */
  /* USART2_RX (in): PA3: IN_FLOATING */
  pinConfig.GPIO_Pin = GPIO_Pin_3;
  pinConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &pinConfig);
  /* USART2_TX (OUT): PA2: Alt_PP */
  pinConfig.GPIO_Pin = GPIO_Pin_2;
  pinConfig.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &pinConfig);

  /* Init USART2 similar to USART1, with lower baudrate */
  initData.USART_BaudRate = 9600;
  USART_Init(USART2, &initData);

  /* Start USART2 */
  USART_Cmd(USART2, ENABLE);
#endif /* MCODE_UART2 */

#endif /* STM32F10X_HD || STM32F10X_MD */

  mcode_scheduler_add (hw_uart_tick);
}

void hw_uart_set_callback(hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void uart_write_char(char ch)
{
  /* Wait until USART1 DR register is empty */
  while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  USART_SendData(USART1, ch);
}

#ifdef MCODE_UART2
void hw_uart2_set_callback(hw_uart_char_event cb)
{
  TheUart2Callback = cb;
}

void uart2_write_char(char ch)
{
  /* Wait until USART1 DR register is empty */
  while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
  USART_SendData(USART2, ch);
}
#endif /* MCODE_UART2 */


static void hw_uart_tick(void)
{
  /* Check if we have any received data */
  if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
    const uint8_t data = USART_ReceiveData(USART1);
    if (TheCallback) {
      (*TheCallback)(data);
    }
  }

#ifdef MCODE_UART2
  /* Check if we have any received data on USART2 */
  if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) != RESET) {
    const uint16_t data = USART_ReceiveData(USART2);
    if (TheUart2Callback) {
      (TheUart2Callback)((char)data);
    }
  }
#endif /* MCODE_UART2 */
}
