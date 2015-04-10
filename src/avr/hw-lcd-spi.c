/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Alexander Chumakov
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

#include "hw-lcd.h"

#include "mtick.h"
#include "hw-spi.h"
#include "hw-uart.h"
#include "console.h"

#include <avr/io.h>

/*
SPI LCD HW configuration
============================
| Pin# | Pin name | uC pin |
============================
|    1 |    +3.3V |    --- |
|    2 |      GND |    --- |
|    3 |       CS |    PB4 |
|    4 |    RESET |    PD6 |
|    5 |      D/C |    PC7 |
|    6 |     MOSI |    PB5 |
|    7 |      SCK |    PB7 |
|    8 |      LED |    --- |
|    9 |     MISO |    PB6 |
============================
*/

void lcd_init(uint16_t width, uint16_t height)
{
  spi_init();
  lcd_set_size(width, height);

  /* Configure D/C (address) line (PC7) as output */
  DDRC |= (1U << DDC7);
  lcd_set_address(true);
  /* Configure RESET line (PD6) as output */
  DDRD |= (1U << DDD6);
  /* Turn RESET line OFF */
  PORTD |= (1U << PD6);

  /* Disable for now, as 'mtick_sleep' does not
     work at this point for some reason */
/*  lcd_reset();*/
}

void lcd_deinit(void)
{
}

void lcd_set_address(bool a0)
{
  if (a0) {
    /* Turn the address line ON (data) */
    PORTC |= (1U << PC7);
  } else {
    /* Turn the address line OFF (command) */
    PORTC &= ~(1U << PC7);
  }
}

void lcd_reset(void)
{
  /* HW reset */
  PORTD &= ~(1U << PD6);
  mtick_sleep(10);
  PORTD |= (1U << PD6);
  mtick_sleep(10);

  /* wait for LCD ready */
  while (0x00009341 != lcd_read_id());

  /* initialize the LCD module */
  lcd_device_init();
}

void lcd_set_size(uint16_t width, uint16_t height)
{
  if (lcd_get_width() != width || lcd_get_height() != height) {
    hw_uart_write_string_P(PSTR("W: lcd_set_size(0x"));
    hw_uart_write_uint16(width, true);
    hw_uart_write_string_P(PSTR(", 0x"));
    hw_uart_write_uint16(height, true);
    hw_uart_write_string_P(PSTR("): unsupported resolution\r\n"));
  }
}
