/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Alexander Chumakov
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

#include <avr/io.h>

static void bl_main_loop(void);
static uint8_t uart_read_char(void);
static void uart_write_char(uint8_t ch);

int main(void)
{
  /* Setup zero register and stack */
  asm volatile ( "eor     r1, r1" );
  asm volatile ( "out     0x3f, r1" );
  asm volatile ( "ldi     r28, 0x5F" );
  asm volatile ( "ldi     r29, 0x08" );
  asm volatile ( "out     0x3e, r29" );
  asm volatile ( "out     0x3d, r28" );

  /* Configure UART */
  /* Set baud rate: 115200 */
  UBRRH = (unsigned char)0;
  UBRRL = (unsigned char)3;
  /* Enable receiver and transmitter */
  UCSRB = (1<<RXEN)|(1<<TXEN);
  /* Set frame format: 8data, 2stop bit */
  UCSRC = (1<<URSEL)|(3<<UCSZ0);

  bl_main_loop();
  return 0;
}

void bl_main_loop(void)
{
  for (;;) {
    const uint8_t ch = uart_read_char();
    uart_write_char(ch);
  }
}

uint8_t uart_read_char(void)
{
  while (!(UCSRA & (1 << RXC)));
  return UDR;
}

void uart_write_char(uint8_t ch)
{
  UDR = ch;
  while (!(UCSRA & (1 << TXC)));
  UCSRA |= (1 << TXC);
}
