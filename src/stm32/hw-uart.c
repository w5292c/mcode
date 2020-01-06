/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 Alexander Chumakov
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

#include "mtick.h"
#include "mstring.h"
#include "scheduler.h"

#include <stddef.h>
#include <string.h>
#include <stm32f10x.h>

#if !defined (STM32F10X_MD) && !defined (STM32F10X_HD)
#error Unsupported device
#endif /* STM32F10X_MD || STM32F10X_HD */

static hw_uart_char_event TheCallback = NULL;
#ifdef MCODE_UART2
#define MCODE_UART_READ_TIMEOUT (100) /*< 100ms */
#define UART2_READ_BUFFERS_COUNT (4)
#define MCODE_UART2_READ_BUFFER_LENGTH (512)

typedef enum {
  ELineReaderStateIdle,
  ELineReaderStateReadingLine,
  ELineReaderStateWaitingN,
} TReaderState;

typedef struct _TLineReaderState {
  uint16_t lineBufferLength[UART2_READ_BUFFERS_COUNT];
  char lineBuffer[UART2_READ_BUFFERS_COUNT][MCODE_UART2_READ_BUFFER_LENGTH];
  /**< The \c readIndex and \c writeIndex size must be enough
       for holding value of \c UART2_READ_BUFFERS_COUNT minus \c 1 */
  uint32_t readIndex : 2;
  uint32_t writeIndex : 2;
  uint32_t state : 4; /**< TReaderState */
  uint32_t ready : 4; /**< Ready indications for each read buffer */
} TLineReaderState;

volatile static TLineReaderState TheUart2State = {0};

static hw_uart_handler TheUart2Callback = NULL;
#endif /* MCODE_UART2 */

static void hw_uart_tick(void);

void hw_uart_init (void)
{
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

  /* Enable the USART2 Interrupt */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  NVIC_InitTypeDef nvicConfig;
  nvicConfig.NVIC_IRQChannel = USART2_IRQn;
  nvicConfig.NVIC_IRQChannelSubPriority = 0;
  nvicConfig.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicConfig);

  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
#endif /* MCODE_UART2 */

  scheduler_add(hw_uart_tick);
}

void hw_uart_set_callback(hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void uart_write_char(char ch)
{
  /* Wait until USART1 DR register is empty */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  USART_SendData(USART1, ch);
}

#ifdef MCODE_UART2
void hw_uart2_set_callback(hw_uart_handler cb)
{
  TheUart2Callback = cb;
}

void uart2_write_char(char ch)
{
  /* Wait until USART1 DR register is empty */
  while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
  USART_SendData(USART2, ch);
}
#endif /* MCODE_UART2 */


static void hw_uart_tick(void)
{
  /* Check if we have any received data in UART1 */
  if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
    const uint8_t data = USART_ReceiveData(USART1);
    if (TheCallback) {
      (*TheCallback)(data);
    }
  }

#ifdef MCODE_UART2
    /* Check if we have a ready line to handle */
  const uint8_t readyFlag = (1u << TheUart2State.readIndex);
  if (!(TheUart2State.ready & readyFlag)) {
    /* No ready data detected */
    return;
  }

  /* Report the currently read line */
  (*TheUart2Callback)((const char *)TheUart2State.lineBuffer[TheUart2State.readIndex],
    TheUart2State.lineBufferLength[TheUart2State.readIndex]);

  /* Reset the reported buffer */
  memset((char *)TheUart2State.lineBuffer[TheUart2State.readIndex], 0, MCODE_UART2_READ_BUFFER_LENGTH);
  TheUart2State.lineBufferLength[TheUart2State.readIndex] = 0;
  /* Ready to read a line in the next buffer, reset the 'ready' flag for the current line */
  ++TheUart2State.readIndex;
  TheUart2State.ready &= ~readyFlag;
#endif /* MCODE_UART2 */
}

#ifdef MCODE_UART2
/* USART2 IRQ handler */
void USART2_IRQHandler(void)
{
  if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    const uint16_t data = USART_ReceiveData(USART2);
    size_t size = TheUart2State.lineBufferLength[TheUart2State.writeIndex];
    volatile char *const buffer = TheUart2State.lineBuffer[TheUart2State.writeIndex];
    switch (TheUart2State.state) {
    case ELineReaderStateIdle:
      size = 0;
      TheUart2State.lineBufferLength[TheUart2State.writeIndex] = 0;
      TheUart2State.state = ELineReaderStateReadingLine;
      /* Fall through to the next state */
    case ELineReaderStateReadingLine:
      if ('\r' == data) {
        /* We are reading the line, and get '\r' as another read character */
        TheUart2State.state = ELineReaderStateWaitingN;
        if (size < MCODE_UART2_READ_BUFFER_LENGTH - 1) {
          /* Check if we have enough space in the buffer for storing the new character */
          /* We reserve 1 byte for the end-of-line indication '\0' */
          buffer[size] = 0;
        } else {
          buffer[MCODE_UART2_READ_BUFFER_LENGTH - 1] = 0;
        }
      } else if (size < MCODE_UART2_READ_BUFFER_LENGTH - 1) {
        /* Check if we have enough space in the buffer for storing the new character */
        /* We reserve 1 byte for the end-of-line indication '\0' */
        buffer[size] = data;
      }
      ++TheUart2State.lineBufferLength[TheUart2State.writeIndex];
      break;
    case ELineReaderStateWaitingN:
      if ('\n' == data) {
        /* Got the right character, switch the buffers, read another line of text */
        /* assert: begin */
        if (TheUart2State.ready & (1u << TheUart2State.writeIndex)) {
          mprintstrln("E: not ready");
        }
        /* assert: end */
        TheUart2State.ready |= (1u << TheUart2State.writeIndex);
        TheUart2State.state = ELineReaderStateIdle;
        ++TheUart2State.writeIndex;
      } else {
        /* Wrong new-line sequence received, reset to idle state, report error */
        mprintstr("E:");
        mprint_uint16(data, true);
        mprint(MStringNewLine);
        TheUart2State.state = ELineReaderStateIdle;
      }
      break;
    }
  }
}
#endif /* MCODE_UART2 */
