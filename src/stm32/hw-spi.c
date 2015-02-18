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

void spi_init(void)
{
  /* GPIO configuration */
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  SPI_InitTypeDef spiConfig;
  spiConfig.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiConfig.SPI_Mode = SPI_Mode_Master;
  spiConfig.SPI_DataSize = SPI_DataSize_8b;
  spiConfig.SPI_CPOL = SPI_CPOL_High;
  spiConfig.SPI_CPHA = SPI_CPHA_2Edge;
  spiConfig.SPI_NSS = SPI_NSS_Soft;
  spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  spiConfig.SPI_FirstBit = SPI_FirstBit_MSB;
  spiConfig.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &spiConfig);

  /* Configure the pins */
  GPIO_InitTypeDef pinConfig;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  /* Configure PB6 pin (SPI_CS) */
  pinConfig.GPIO_Pin = GPIO_Pin_6;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &pinConfig);
  /* Configure PA7 pin (SPI_MOSI) */
  pinConfig.GPIO_Pin = GPIO_Pin_7;
  pinConfig.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &pinConfig);
  /* Configure PA5 pin (SPI_SCK) */
  pinConfig.GPIO_Pin = GPIO_Pin_5;
  pinConfig.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &pinConfig);
  /* Configure PA6 pin (SPI_MISO) */
  pinConfig.GPIO_Pin = GPIO_Pin_6;
  pinConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &pinConfig);

  SPI_Cmd(SPI1, ENABLE);
}

void spi_deinit(void)
{
}

void spi_set_cs(bool selected)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_6, selected ? Bit_RESET : Bit_SET);
}

uint8_t spi_transfer(uint8_t data)
{
  SPI_I2S_SendData(SPI1, data);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) != RESET) {}
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {}
  const uint8_t byte = SPI_I2S_ReceiveData(SPI1);
  return byte;
}
