/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Alexander Chumakov
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

#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

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

static void hw_uart_tick(void);

void hw_uart_init(void)
{
  TheCurrentBuffer = 0;
  TheCurrentReadIndex0 = 0;
  TheCurrentReadIndex1 = 0;
  memset((void *)TheReadBuffer0, 0, HW_UART_READ_BUFFER_LENGTH);
  memset((void *)TheReadBuffer1, 0, HW_UART_READ_BUFFER_LENGTH);

  /* Set baud rate: 115200 */
  UBRRH = (unsigned char)0;
  UBRRL = (unsigned char)3;
  UCSRA = 0;
  /* Enable receiver and transmitter */
  UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
  /* Set frame format: 8data, 1stop bit */
  UCSRC = (1<<URSEL)|(3<<UCSZ0);

  mcode_scheduler_add(hw_uart_tick);
}

void hw_uart_set_callback(hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void uart_write_char(char ch)
{
  UDR = ch;
  loop_until_bit_is_set(UCSRA, UDRE);
}

static void hw_uart_tick(void)
{
  if (TheCurrentBuffer) {
    if (TheCurrentReadIndex1) {
      /* disable RXC interrupt */
      UCSRB &= ~(1<<RXCIE);

      /* toggle the current buffer */
      TheCurrentBuffer = 0;

      /* enable interrupt again */
      UCSRB |= (1<<RXCIE);

      /* */
      const uint8_t length = TheCurrentReadIndex1;
      TheCurrentReadIndex1 = 0;
      if (TheCallback) {
        uint8_t i;
        for (i = 0; i < length; ++i) {
          (*TheCallback)(TheReadBuffer1[i]);
        }
      }
    }
  } else {
    if (TheCurrentReadIndex0) {
      /* disable RXC interrupt */
      UCSRB &= ~(1<<RXCIE);

      /* toggle the current buffer */
      TheCurrentBuffer = 1;

      /* enable interrupt again */
      UCSRB |= (1<<RXCIE);

      /* */
      const uint8_t length = TheCurrentReadIndex0;
      TheCurrentReadIndex0 = 0;
      if (TheCallback) {
        uint8_t i;
        for (i = 0; i < length; ++i) {
          (*TheCallback)(TheReadBuffer0[i]);
        }
      }
    }
  }
}

/* USART, Rx Complete */
ISR(USART_RXC_vect)
{
  /* read the received byte */
  const uint8_t data = UDR;

  if (TheCurrentBuffer) {
    if (TheCurrentReadIndex1 < (HW_UART_READ_BUFFER_LENGTH - 1)) {
      /* there is enough space in the read buffer to store the received byte */
      TheReadBuffer1[TheCurrentReadIndex1++] = data;
      /**@todo check if we need to handle the case when we ran out of the read buffer */
    }
  } else {
    if (TheCurrentReadIndex0 < (HW_UART_READ_BUFFER_LENGTH - 1)) {
      /* there is enough space in the read buffer to store the received byte */
      TheReadBuffer0[TheCurrentReadIndex0++] = data;
      /**@todo check if we need to handle the case when we ran out of the read buffer */
    }
  }
}
