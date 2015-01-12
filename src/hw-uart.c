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

#include "hw-uart.h"

#include "scheduler.h"
#include "mcode-config.h"
#include "line-editor-uart.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef MCODE_EMULATE_UART

#ifdef __AVR__
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#else /* __AVR__ */
#include "emu-common.h"
#include <stdint.h>
#include <stm32f10x.h>
#endif /* __AVR__ */

static hw_uart_char_event TheCallback = NULL;

/**
 * TheCurrentBuffer == 0: reading from UART to TheReadBuffer0, keeping TheReadBuffer1
 * TheCurrentBuffer == 1: reading from UART to TheReadBuffer1, keeping TheReadBuffer0
 */
#define HW_UART_READ_BUFFER_LENGTH (16)
volatile static uint8_t TheCurrentBuffer = 0;
volatile static uint8_t TheCurrentReadIndex0 = 0;
volatile static uint8_t TheCurrentReadIndex1 = 0;
volatile static uint8_t TheReadBuffer0[HW_UART_READ_BUFFER_LENGTH];
volatile static uint8_t TheReadBuffer1[HW_UART_READ_BUFFER_LENGTH];

#ifndef MCODE_HW_UART_SYNC_WRITE
#define HW_UART_WRITE_BUFFER_LENGTH (128)
volatile static unsigned int TheWriteBufferEnd = 0;
volatile static unsigned int TheWriteBufferStart = 0;
volatile static unsigned char TheWriteBuffer[HW_UART_WRITE_BUFFER_LENGTH];
#endif /* MCODE_HW_UART_SYNC_WRITE */

static void hw_uart_tick (void);

void hw_uart_init (void)
{
  TheCurrentBuffer = 0;
  TheCurrentReadIndex0 = 0;
  TheCurrentReadIndex1 = 0;
  memset ((void *)TheReadBuffer0, 0, HW_UART_READ_BUFFER_LENGTH);
  memset ((void *)TheReadBuffer1, 0, HW_UART_READ_BUFFER_LENGTH);
#ifndef MCODE_HW_UART_SYNC_WRITE
  TheWriteBufferEnd = 0;
  TheWriteBufferStart = 0;
  memset (TheWriteBuffer, 0, HW_UART_WRITE_BUFFER_LENGTH);
#endif /* MCODE_HW_UART_SYNC_WRITE */

#ifdef __AVR_MEGA__
  /* Set baud rate: 115200 */
  UBRRH = (unsigned char)0;
  UBRRL = (unsigned char)3;
  /* Enable receiver and transmitter */
  UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
  /* Set frame format: 8data, 2stop bit */
  UCSRC = (1<<URSEL)|(3<<UCSZ0);
#endif /* __AVR_MEGA__ */

#ifdef STM32F10X_HD
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
#endif /* STM32F10X_HD */

  mcode_scheduler_add (hw_uart_tick);
}

void hw_uart_deinit (void)
{
}

void hw_uart_set_callback (hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void hw_uart_start_read (void)
{
}

static int8_t vtoch (uint8_t value)
{
  value = value & 0x0FU;
  if (value < 10 && value >= 0)
  {
    return '0' + value;
  }
  else if (value >= 10 && value < 16)
  {
    return 'A' + value - 10;
  }
  else
  {
    return '@';
  }
}


void hw_uart_write_uint(uint16_t value)
{
  hw_uart_write_uint16(value, false);
}

void hw_uart_write_uint64(uint64_t value, bool skipZeros)
{
  const uint32_t upper = (uint32_t)(value>>32);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    hw_uart_write_uint32(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  hw_uart_write_uint32((uint32_t)value, skipZeros);
}

void hw_uart_write_uint32(uint32_t value, bool skipZeros)
{
  const uint16_t upper = (uint16_t)(value>>16);
  if (!skipZeros || 0 != upper) {
    /* skip upper part if it is empty */
    hw_uart_write_uint16(upper, skipZeros);
    /* if the upper part is not empty, we cannot skip zeroes any longer */
    skipZeros = false;
  }
  hw_uart_write_uint16((uint16_t)value, skipZeros);
}

void hw_uart_write_uint16(uint16_t value, bool skipZeros)
{
  int i;
  char buffer[5];
  buffer[0] = vtoch (0x0FU & (value >> 12));
  buffer[1] = vtoch (0x0FU & (value >>  8));
  buffer[2] = vtoch (0x0FU & (value >>  4));
  buffer[3] = vtoch (0x0FU & value);
  buffer[4] = 0;
  if (skipZeros) {
    for (i = 0; i < 3; ++i) {
      if ('0' == *buffer) {
        memmove(buffer, buffer + 1, 4);
      }
    }
  }
  hw_uart_write_string(buffer);
}

void hw_uart_write_string (const char *aString)
{
#ifdef MCODE_HW_UART_SYNC_WRITE

#ifdef __AVR__
  uint8_t ch;
  while (0 != (ch = *aString++))
  {
    while ( !( UCSRA & (1<<UDRE)) ) ;
    UDR = ch;
  }
#endif /* __AVR__ */

#ifdef STM32F10X_HD
  uint8_t ch;
  while (0 != (ch = *aString++)) {
    /* Wait until USART1 DR register is empty */
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, ch);
  }
#endif /* STM32F10X_HD */

#else /* MCODE_HW_UART_SYNC_WRITE */
  if (TheWriteBufferStart)
  {
    memmove (&TheWriteBuffer[0], &TheWriteBuffer[TheWriteBufferStart], TheWriteBufferStart);

    TheWriteBufferEnd -= TheWriteBufferStart;
    TheWriteBufferStart = 0;
  }

  if (length)
  {
    const int freeBufferLength = HW_UART_WRITE_BUFFER_LENGTH - (TheWriteBufferEnd - TheWriteBufferStart);
    if (length > freeBufferLength)
    {
      length = freeBufferLength;
    }

    memcpy (&TheWriteBuffer[TheWriteBufferEnd], aString, length);
    TheWriteBufferEnd += length;
  }
#endif /* MCODE_HW_UART_SYNC_WRITE */
}

void hw_uart_write_string_P (const char *aString)
{
#ifdef MCODE_HW_UART_SYNC_WRITE

#ifdef __AVR__
  uint8_t ch;
  while (0 != (ch = pgm_read_byte (aString++)))
  {
    while ( !( UCSRA & (1<<UDRE)) );
    UDR = ch;
  }
#else /* __AVR__ */
  hw_uart_write_string(aString);
#endif /* __AVR__ */

#else /* MCODE_HW_UART_SYNC_WRITE */
  /**@todo implement async version */
#endif /* MCODE_HW_UART_SYNC_WRITE*/
}

static void hw_uart_tick (void)
{
#ifdef __AVR__
  if (TheCurrentBuffer)
  {
    if (TheCurrentReadIndex1)
    {
      /* disable RXC interrupt */
      UCSRB &= ~(1<<RXCIE);

      /* toggle the current buffer */
      TheCurrentBuffer = /*!TheCurrentBuffer*/0;

      /* enable interrupt again */
      UCSRB |= (1<<RXCIE);

      /* */
      if (TheCallback)
      {
        uint8_t i;
        for (i = 0; i < TheCurrentReadIndex1; ++i)
        {
          (*TheCallback) (TheReadBuffer1[i]);
        }
      }

      /* we passed the content of the buffer to the client, reset the index */
      TheCurrentReadIndex1 = 0;
    }
  }
  else
  {
    if (TheCurrentReadIndex0)
    {
      /* disable RXC interrupt */
      UCSRB &= ~(1<<RXCIE);

      /* toggle the current buffer */
      TheCurrentBuffer = /*!TheCurrentBuffer*/1;

      /* enable interrupt again */
      UCSRB |= (1<<RXCIE);

      /* */
      if (TheCallback)
      {
        uint8_t i;
        for (i = 0; i < TheCurrentReadIndex0; ++i)
        {
          (*TheCallback) (TheReadBuffer0[i]);
        }
      }

      /* we passed the content of the buffer to the client, reset the index */
      TheCurrentReadIndex0 = 0;
    }
  }

#endif /* __AVR__ */

#ifdef STM32F10X_HD
  /* Check if we have any received data */
  if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
    const uint8_t data = USART_ReceiveData(USART1);
#if 0
    hw_uart_write_string("Received: ");
    hw_uart_write_uint(data);
    hw_uart_write_string("\r\n");
#endif /* 0 */
    if (TheCallback) {
      (*TheCallback)(data);
    }
  }
#endif /* */

#ifndef MCODE_HW_UART_SYNC_WRITE
  const int bufferBytes = (TheWriteBufferEnd - TheWriteBufferStart);
  if (bufferBytes)
  {
#if 0
    fprintf (stdout, PSTR("%c"), (char)TheWriteBuffer[TheWriteBufferStart++]);
    fflush (stdout);
#endif /* 0 */
  }
#endif /* MCODE_HW_UART_SYNC_WRITE */

#if 0 /* test code */
  static int n = 0;
  if (++n == 20)
  {
    hw_uart_write_string_P (PSTR("2 secs passed.\n"));
    line_editor_uart_start ();
    n = 0;
  }
#endif /* test code */
}

#ifdef __AVR_ATmega32__
/* USART, Rx Complete */
ISR(USART_RXC_vect)
{
  /* read the received byte */
  const uint8_t data = UDR;

  if (TheCurrentBuffer)
  {
    if (TheCurrentReadIndex1 < (HW_UART_READ_BUFFER_LENGTH - 1))
    {
      /* there is enough space in the read buffer to store the received byte */
      TheReadBuffer1[TheCurrentReadIndex1++] = data;
      /**@todo check if we need to handle the case when we ran out of the read buffer */
    }
  }
  else
  {
    if (TheCurrentReadIndex0 < (HW_UART_READ_BUFFER_LENGTH - 1))
    {
      /* there is enough space in the read buffer to store the received byte */
      TheReadBuffer0[TheCurrentReadIndex0++] = data;
      /**@todo check if we need to handle the case when we ran out of the read buffer */
    }
  }
}

#if 0 /**@todo check if we need to remove these 2 ISRs */
/* USART Data Register Empty */
ISR(USART_UDRE_vect)
{
}

/* USART, Tx Complete */
ISR(USART_TXC_vect)
{
}
#endif /* 0 */

#endif /* __AVR_ATmega32__ */

#else /* MCODE_EMULATE_UART */
#include "emu-hw-uart.h"
/* for emulation, just forward the requests to the corresponding emulator implementation */
void hw_uart_init (void) { emu_hw_uart_init (); }
void hw_uart_deinit (void) { emu_hw_uart_deinit (); }
void hw_uart_set_callback (hw_uart_char_event aCallback) { emu_hw_uart_set_callback (aCallback); }
void hw_uart_start_read (void) { emu_hw_uart_start_read (); }
void hw_uart_write_uint (unsigned int value) { emu_hw_uart_write_uint (value); }
void hw_uart_write_string (const char *aString) { emu_hw_uart_write_string (aString); }
void hw_uart_write_string_P (const char *aString) { emu_hw_uart_write_string_P (aString); }
#endif /* MCODE_EMULATE_UART */
