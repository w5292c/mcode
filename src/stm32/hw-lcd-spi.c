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

#include "mtick.h"
#include "hw-uart.h"

#include <stdarg.h>
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

static void lcd_spi_init(void);
static void lcd_write_cmd(uint8_t cmd);
static void lcd_write_byte(uint8_t data);
static void lcd_spi_set_cs(bool selected);
static void lcd_spi_set_command(bool cmd);
static uint8_t lcd_read_register(uint8_t addr, uint8_t param);

#define INLINE_BYTES(str) ((const uint8 *)str)

void lcd_init(uint16_t width, uint16_t height)
{
  /* GPIO configuration */
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
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
  spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  spiConfig.SPI_FirstBit = SPI_FirstBit_MSB;
  spiConfig.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &spiConfig);

  /* Configure the pins */
  GPIO_InitTypeDef pinConfig;
  /* Configure PD4 pin (LED) */
  pinConfig.GPIO_Pin =  GPIO_Pin_4;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_OD;
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

  lcd_reset();
  while (0x00009341 != lcd_read_id());

  lcd_spi_init();

  /* Handle the current LCD size */
  lcd_set_size(width, height);
}

void lcd_deinit(void)
{
}

void lcd_reset(void)
{
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_RESET);
  mtick_sleep(10);
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
  mtick_sleep(10);
}

void lcd_set_read_cb(lcd_read_cb cb)
{
  TheReadCallback = cb;
}

void lcd_read(uint8_t cmd, uint8_t length)
{
}

void lcd_set_bl(bool on)
{
  GPIO_WriteBit(GPIOD, GPIO_Pin_4, on ? Bit_RESET : Bit_SET);
}

uint8_t spi_transfer(uint8_t data)
{
  SPI_I2S_SendData(SPI1, data);
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) != RESET) {}
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {}
  const uint8_t byte = SPI_I2S_ReceiveData(SPI1);
  return byte;
}

uint16_t lcd_get_width(void)
{
  return 240;
}

uint16_t lcd_get_height(void)
{
  return 320;
}

void lcd_set_size(uint16_t width, uint16_t height)
{
  if (width != lcd_get_width() || height != lcd_get_height()) {
    hw_uart_write_string("W: lcd_set_size(0x");
    hw_uart_write_uint16(width, true);
    hw_uart_write_string(", 0x");
    hw_uart_write_uint16(height, true);
    hw_uart_write_string(")\r\n");
  }
}

uint32_t lcd_read_id(void)
{
  int i;
  uint8_t id[3] = {0};
  for (i = 0; i < 3; ++i) {
    id[i] = lcd_read_register(0xd3, i + 1);
  }

  const uint32_t idValue = (id[0] << 16) | (id[1] << 8) | id[2];
  return idValue;
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

  lcd_cls(0x0000);
}

void lcd_spi_set_cs(bool selected)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_6, selected ? Bit_SET : Bit_RESET);
}

void lcd_spi_set_command(bool cmd)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_7, cmd ? Bit_SET : Bit_RESET);
}

void lcd_write_cmd(uint8_t cmd)
{
  lcd_spi_set_command(false);
  lcd_spi_set_cs(false);
  spi_transfer(cmd);
  lcd_spi_set_cs(true);
}

void lcd_write_byte(uint8_t data)
{
  lcd_spi_set_command(true);
  lcd_spi_set_cs(false);
  spi_transfer(data);
  lcd_spi_set_cs(true);
}

uint8_t lcd_read_register(uint8_t addr, uint8_t param)
{
  uint8_t data = 0;
  lcd_write_cmd(0xD9U);
  lcd_write_byte(0x10 + param);
  lcd_spi_set_command(false);
  lcd_spi_set_cs(false);
  spi_transfer(addr);
  lcd_spi_set_command(true);
  data = spi_transfer(0xffu);
  lcd_spi_set_cs(true);
  return data;
}

void lcd_write(int len, ...)
{
  va_list vl;
  va_start(vl, len);
  lcd_write_cmd(va_arg(vl, unsigned int));
  for (int i = 1; i < len; ++i) {
    lcd_write_byte(va_arg(vl, unsigned int));
  }
  va_end(vl);
}

void lcd_set_scroll_start(uint16_t start)
{
  lcd_command(0x37, start>>8, start);
}

void lcd_set_window(uint16_t colStart, uint16_t colEnd, uint16_t rowStart, uint16_t rowEnd)
{
  lcd_command(0x2A, colStart>>8, colStart, colEnd>>8, colEnd);
  lcd_command(0x2B, rowStart>>8, rowStart, rowEnd>>8, rowEnd);
}

void lcd_write_const_words(uint8_t cmd, uint16_t word, uint32_t count)
{
  const uint8_t byte1 = word>>8;
  const uint8_t byte2 = word;
  lcd_write_cmd(cmd);
  for (uint32_t i = 0; i < count; ++i) {
    lcd_write_byte(byte1);
    lcd_write_byte(byte2);
  }
}

void lcd_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  uint8_t bitMask;
  uint8_t currentByte;
  const uint8_t *const pDataEnd = pData + length;

  lcd_write_cmd(cmd);
  /* write loop */
  currentByte = *pData++;
  for (bitMask = UINT8_C (0x01); ; ) {
    const uint16_t currentData = (currentByte & bitMask) ? onValue : offValue;
    lcd_write_byte(currentData>>8);
    lcd_write_byte(currentData);
    bitMask = (bitMask << 1);
    if (!bitMask) {
      if (pData < pDataEnd) {
        bitMask = UINT8_C (0x01);
        currentByte = *pData++;
      }
      else {
        break;
      }
    }
  }
}

void lcd_turn(bool on)
{
  lcd_command(on ? 0x29 : 0x28);
}
