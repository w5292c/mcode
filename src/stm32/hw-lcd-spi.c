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

#include "hw-lcd.h"

#include <stm32f10x.h>

/*
 * SPI LCD HW configuration:
 * 4. RESET;    GPIO;      PD5;
 * 5. D/C;      GPIO;      PB7;
 * 8. LED;      GPIO/NONE; PD4;
 * 3. SPI_CS;   GPIO;      PB6;
 * 6. SPI_MOSI; SPI;       PA7;
 * 7. SPI_SCK;  SPI;       PA5;
 * 9. SPI_MISO; SPI;       PA6;
 * 1. Vcc (+3.3V);
 * 2. GND;
 */
static lcd_read_cb TheReadCallback = 0;

void lcd_init(void)
{
  /* GPIO configuration */
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

  /* Configure the pins */
  GPIO_InitTypeDef pinConfig;
  /* Configure PD4 pin (LED) */
  pinConfig.GPIO_Pin =  GPIO_Pin_4;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &pinConfig);
  GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);
  /* Configure PD5 pin (RESET) */
  pinConfig.GPIO_Pin = 5;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &pinConfig);
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
  /* Configure PB7 pin (D/C) */
  pinConfig.GPIO_Pin = 7;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &pinConfig);
  GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET);
  /* Configure PB6 pin (SPI_CS) */
  pinConfig.GPIO_Pin = 6;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &pinConfig);
  GPIO_WriteBit(GPIOB, GPIO_Pin_6, Bit_SET);
  /* Configure PA7 pin (SPI_MOSI) */
  pinConfig.GPIO_Pin = 7;
  pinConfig.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &pinConfig);
  /* Configure PA5 pin (SPI_SCK) */
  pinConfig.GPIO_Pin = 5;
  pinConfig.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &pinConfig);
  /* Configure PA6 pin (SPI_MISO) */
  pinConfig.GPIO_Pin = 6;
  pinConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &pinConfig);
}

void lcd_deinit(void)
{
}

void lcd_reset(void)
{
  volatile uint32_t i;
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_RESET);
  for (i = 0; i < 0x1FFFFU; ++i);
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
  for (i = 0; i < 0x1FFFFU; ++i);
}

void lcd_set_read_cb(lcd_read_cb cb)
{
  TheReadCallback = cb;
}

void lcd_read(uint8_t cmd, uint8_t length)
{
}

void lcd_write(uint8_t cmd, uint8_t length, const uint8_t *data)
{
}

void lcd_set_bl(bool on)
{
  GPIO_WriteBit(GPIOD, GPIO_Pin_4, on ? Bit_SET : Bit_RESET);
}
