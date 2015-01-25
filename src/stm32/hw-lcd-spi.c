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

#include "hw-uart.h"

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
/*static uint32_t TheLcdId = 0xFF00AA55U;*/

static void lcd_spi_set_cs(bool selected);
static void lcd_spi_set_command(bool cmd);

void lcd_init(void)
{
  /* GPIO configuration */
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  /*RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI1, ENABLE);*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

  SPI_InitTypeDef spiConfig;
  spiConfig.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiConfig.SPI_Mode = SPI_Mode_Master;
  spiConfig.SPI_DataSize = SPI_DataSize_8b;
  spiConfig.SPI_CPOL = SPI_CPOL_High;
  spiConfig.SPI_CPHA = SPI_CPHA_2Edge;
  spiConfig.SPI_NSS = SPI_NSS_Soft;
  spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
  spiConfig.SPI_FirstBit = SPI_FirstBit_MSB;
  spiConfig.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &spiConfig);

  /* Configure the pins */
  GPIO_InitTypeDef pinConfig;
  /* Configure PD4 pin (LED) */
  pinConfig.GPIO_Pin =  GPIO_Pin_4;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &pinConfig);
  GPIO_WriteBit(GPIOD, GPIO_Pin_4, Bit_RESET);
  /* Configure PD5 pin (RESET) */
  pinConfig.GPIO_Pin = GPIO_Pin_5;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &pinConfig);
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
  /* Configure PB7 pin (D/C) */
  pinConfig.GPIO_Pin = GPIO_Pin_7;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &pinConfig);
  GPIO_WriteBit(GPIOB, GPIO_Pin_7, Bit_RESET);
  /* Configure PB6 pin (SPI_CS) */
  pinConfig.GPIO_Pin = GPIO_Pin_6;
  pinConfig.GPIO_Mode = GPIO_Mode_AF_PP;
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

uint8_t spi_transfer(uint8_t data)
{
  SPI_I2S_SendData(SPI1, data);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != RESET) {}
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {}
  uint16_t dd = SPI_I2S_ReceiveData(SPI1);
  return dd | dd>>8;
}

#if 1
void write_data(uint8_t data)
{
  lcd_spi_set_command(true);
  lcd_spi_set_cs(false);
  spi_transfer(data);
  lcd_spi_set_cs(true);
}

void send_cmd(uint8_t cmd)
{
  lcd_spi_set_command(false);
  lcd_spi_set_cs(false);
  spi_transfer(cmd);
  lcd_spi_set_cs(true);
}

uint8_t read_register(uint8_t addr, uint8_t param)
{
  uint8_t data = 0;
  send_cmd(0xD9U);
  write_data(0x10 + param);
  lcd_spi_set_command(false); /* TFT_DC_LOW */
  lcd_spi_set_cs(false);  /* TFT_CS_LOW */
  spi_transfer(addr);
  lcd_spi_set_command(true);  /* TFT_DC_HIGH */
  data = spi_transfer(0xffu);
  lcd_spi_set_cs(true);   /* TFT_CS_HIGH */
  return data;
}
#endif /* 1 */

uint32_t lcd_read_id(void)
{
  int i;
  uint8_t id[4] = {0};
  for (i = 0; i < 3; ++i) {
    id[i] = read_register(0xd3, i + 1);
  }

  const uint32_t idValue = id[0] | (id[1] << 8) | (id[2] << 16) | (id[3] << 24);
  return idValue;
}

void lcd_spi_set_cs(bool selected)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_6, selected ? Bit_SET : Bit_RESET);
}

void lcd_spi_set_command(bool cmd)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_7, cmd ? Bit_SET : Bit_RESET);
}
