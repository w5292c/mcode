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
|    4 |    RESET |    PD7 |
|    5 |      D/C |    PC7 |
|    6 |     MOSI |    PB5 |
|    7 |      SCK |    PB7 |
|    8 |      LED |    --- |
|    9 |     MISO |    PB6 |
============================
*/

static void lcd_spi_init(void);

void lcd_init(uint16_t width, uint16_t height)
{
  spi_init();

  /* Configure D/C (address) line (PC7) as output */
  DDRC |= (1U << DDC7);
  lcd_set_address(true);
  /* Configure RESET line (PD7) as output */
  DDRD |= (1U << DDD7);
  /* Turn RESET line OFF */
  PORTD |= (1U << PD7);

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
  PORTD &= ~(1U << PD7);
  mtick_sleep(10);
  PORTD |= (1U << PD7);
  mtick_sleep(10);

  /* wait for LCD ready */
  while (0x00009341 != lcd_read_id());

  /* initialize the LCD module */
  lcd_spi_init();
}

void lcd_spi_init(void)
{
 /* SW reset */
  lcd_command(0x01);
  mtick_sleep(5);
  /* Display OFF */
  lcd_command(0x28);
  /* Power control A: default */
  lcd_command(0xCB, 0x39, 0X2C, 0x00, 0x34, 0x02);
  /* Power control B: default */
  lcd_command(0xCF, 0x00, 0x83, 0x30);
  /* Power on sequence control */
  lcd_command(0xED, 0x64, 0x03, 0x12, 0x81);
  /* Driver timing control A */
  lcd_command(0xE8, 0x85, 0x01, 0x79);
  /* Driver timing control B */
  lcd_command(0xEA, 0x00, 0x00);
  /* Pump ratio control */
  lcd_command(0xF7, 0x20);
  /* Power control */
  lcd_command(0xC0, 0x26);
  lcd_command(0xC1, 0x11);
  /* VCOM */
  lcd_command(0xC5, 0x35, 0x3E);
  lcd_command(0xC7, 0xBE);
  /* Memory access control: 16 bits per pixel */
  lcd_command(0x3A, 0x55);
  /* Memory Access Control */
  lcd_command(0x36U, 0x48);
  /* Frame rate */
  lcd_command(0xB1, 0x00, 0x1B);
  /* Gamma set */
  lcd_command(0x26, 0x01);
  /* Entry Mode Set */
  lcd_command(0xB7, 0x07);
  /* Display Function Control */
  lcd_command(0xB6, 0x0A, 0x82, 0x27, 0x00);
  /* Exit Sleep */
  lcd_command(0x11);
  mtick_sleep(100);
  /* Display: ON */
  lcd_command(0x29);
  mtick_sleep(20);

#if 1 /* clear screen in console code */
  console_set_color(UINT16_C(0xFFFF));
  console_set_bg_color(UINT16_C(0x0000));
  console_clear_screen();
#else /* clear screen in console code */
  lcd_cls(0x0000);
#endif /* clear screen in console code */
}
