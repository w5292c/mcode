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

#include "hw-spi.h"

#include "hw-uart.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

/*!
 * SPI HW configuration:
 * ===============================
 * | Pin# | Pin name | Port name |
 * ===============================
 * |    5 | SPI_CS   | PB4       |
 * |    6 | SPI_MOSI | PB5       |
 * |    7 | SPI_MISO | PB6       |
 * |    8 | SPI_SCK  | PB7       |
 * ===============================
 */

void spi_init(void)
{
  /* First, configure CS as output */
  /* This MUST be done BEFORE configuring SPI as a master */
  DDRB |= (1U<<DDB4);
  spi_set_cs(false);
  /* Enable SPI, Master, set clock rate fck/2 */
  SPCR = (1U<<SPE) | (1U<<MSTR);
  /* Reset SPI2X flag */
  SPSR |= (1U<<SPI2X);
  /* Set SS, MOSI and SCK output */
  DDRB |= (1U<<DDB4) | (1U<<DDB5) | (1U<<DDB7);
  /* Set MISO input */
  DDRB &= ~(1U<<DDB6);
}

void spi_deinit(void)
{
}

void spi_set_cs(bool selected)
{
  if (!selected) {
    PORTB |= (1U<<PB4);
  } else {
    PORTB &= ~(1U<<PB4);
  }
}

uint8_t spi_transfer(uint8_t data)
{
  /* Start transmission */
  SPDR = data;
  /* Wait for transmission complete */
  while(!(SPSR & (1<<SPIF)));
  /* Now, read the read byte */
  data = SPDR;
  return data;
}
