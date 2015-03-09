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
#include "hw-spi.h"
#include "hw-uart.h"
#include "console.h"

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

static void lcd_spi_init(void);
static void lcd_write_cmd(uint8_t cmd);
static void lcd_write_byte(uint8_t data);
static void lcd_spi_set_command(bool cmd);
static uint8_t lcd_read_register(uint8_t addr, uint8_t param);

#define INLINE_BYTES(str) ((const uint8 *)str)

void lcd_init(uint16_t width, uint16_t height)
{
  /* Initialize SPI first */
  spi_init();

  /* GPIO configuration */
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

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

  /* Handle the current LCD size */
  lcd_set_size(width, height);
  lcd_reset();
}

void lcd_deinit(void)
{
  spi_deinit();
}

void lcd_reset(void)
{
  /* HW reset */
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_RESET);
  mtick_sleep(10);
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
  mtick_sleep(10);

  /* wait for LCD ready */
  while (0x00009341 != lcd_read_id());

  /* initialize the LCD module */
  lcd_spi_init();
}

void lcd_read(uint8_t cmd, uint8_t length, uint8_t *data)
{
}

void lcd_set_bl(bool on)
{
  GPIO_WriteBit(GPIOD, GPIO_Pin_4, on ? Bit_RESET : Bit_SET);
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

#if 1 /* clear screen in console code */
  console_set_color(UINT16_C(0xFFFF));
  console_set_bg_color(UINT16_C(0x0000));
  console_clear_screen();
#else /* clear screen in console code */
  lcd_cls(0x0000);
#endif /* clear screen in console code */
}

void lcd_spi_set_command(bool cmd)
{
  GPIO_WriteBit(GPIOB, GPIO_Pin_7, cmd ? Bit_SET : Bit_RESET);
}

void lcd_write_cmd(uint8_t cmd)
{
  lcd_spi_set_command(false);
  spi_set_cs(true);
  spi_transfer(cmd);
  spi_set_cs(false);
}

void lcd_write_byte(uint8_t data)
{
  lcd_spi_set_command(true);
  spi_set_cs(true);
  spi_transfer(data);
  spi_set_cs(false);
}

uint8_t lcd_read_register(uint8_t addr, uint8_t param)
{
  uint8_t data = 0;
  lcd_write_cmd(0xD9U);
  lcd_write_byte(0x10 + param);
  lcd_spi_set_command(false);
  spi_set_cs(true);
  spi_transfer(addr);
  lcd_spi_set_command(true);
  data = spi_transfer(0xffu);
  spi_set_cs(false);
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
