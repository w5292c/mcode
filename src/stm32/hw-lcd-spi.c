/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2017 Alexander Chumakov
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
#include "mstring.h"
#include "mglobal.h"

#include <stm32f10x.h>

/*
 * SPI LCD HW configuration:
 * 4. RESET;    GPIO;      PD5; PA4 (STM32F103C8);
 * 5. D/C;      GPIO;      PA1;
 * 8. LED;      GPIO/NONE; PD4; NONE (STM32F103C8);
 * 3. SPI_CS;   GPIO;      PA0;
 * 6. SPI_MOSI; SPI;       PA7;
 * 7. SPI_SCK;  SPI;       PA5;
 * 9. SPI_MISO; SPI;       PA6;
 * 1. Vcc (+3.3V);
 * 2. GND;
 */

#if !defined (STM32F10X_HD) && !defined (STM32F10X_MD)
#error "Unsupported device"
#endif /* !STM32F10X_HD && !STM32F10X_MD */

/**
 * Connect LCD_DC pin to PA1
 */
#define MCODE_LCD_DC_PORT GPIOA
#define MCODE_LCD_DC_PIN GPIO_Pin_1

void lcd_init(uint16_t width, uint16_t height)
{
  /* Initialize SPI first */
  spi_init();

  /* GPIO configuration */
  /* Enable clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
#ifdef STM32F10X_HD
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
#elif defined (STM32F10X_MD)
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
#endif /* STM32F10X_HD */

#ifdef STM32F10X_MD
  /* Keep SWJ enabled, but disable JTAG interface */
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
#endif /* STM32F10X_MD */

  /* Configure the pins */
  GPIO_InitTypeDef pinConfig;
#ifdef STM32F10X_HD
  /* Configure the LCD-BACKLIGHT pin */
  pinConfig.GPIO_Pin =  GPIO_Pin_4;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_OD;
  pinConfig.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &pinConfig);
#elif defined (STM32F10X_MD)
  pinConfig.GPIO_Pin =  GPIO_Pin_15;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_OD;
  GPIO_Init(GPIOA, &pinConfig);
#endif /* STM32F10X_HD || STM32F10X_MD */
  /* Turn the backlight OFF on default */
  lcd_set_bl(false);

#ifdef STM32F10X_HD
  /* Configure PD5 pin (RESET) for high-density devices */
  pinConfig.GPIO_Pin = GPIO_Pin_5;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &pinConfig);
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
#elif defined (STM32F10X_MD)
  /* Configure RESET pin (PA4) for medium-density devices */
  pinConfig.GPIO_Pin = GPIO_Pin_4;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &pinConfig);
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
#endif /* STM32F10X_HD || STM32F10X_MD */
  /* Configure PA1 pin (D/C) */
  pinConfig.GPIO_Pin = MCODE_LCD_DC_PIN;
  pinConfig.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(MCODE_LCD_DC_PORT, &pinConfig);
  GPIO_WriteBit(MCODE_LCD_DC_PORT, MCODE_LCD_DC_PIN, Bit_RESET);

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
  int i;
  uint32_t id;

  /* HW reset */
#ifdef STM32F10X_HD
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_RESET);
#elif defined (STM32F10X_MD)
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_RESET);
#endif /* STM32F10X_HD || STM32F10X_MD */
  mtick_sleep(10);
#ifdef STM32F10X_HD
  GPIO_WriteBit(GPIOD, GPIO_Pin_5, Bit_SET);
#elif defined (STM32F10X_MD)
  GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
#endif /* STM32F10X_HD || STM32F10X_MD */
  mtick_sleep(10);

  /* wait for LCD ready */
  for (i = 0; i < 3; ++i) {
    id = lcd_read_id();
    if (0x00009341 == id) {
      break;
    } else {
      mprintstr("Error: failed reading LCD ID, got: ");
      mprint_uint32(id, false);
      mprint(MStringNewLine);
      mtick_sleep(500);
    }
  }

  /* initialize the LCD module */
  lcd_device_init();
}

void lcd_set_bl(bool on)
{
#ifdef STM32F10X_HD
  GPIO_WriteBit(GPIOD, GPIO_Pin_4, on ? Bit_RESET : Bit_SET);
#elif defined (STM32F10X_MD)
  GPIO_WriteBit(GPIOA, GPIO_Pin_15, on ? Bit_RESET : Bit_SET);
#endif /* STM32F10X_HD || STM32F10X_MD */
}

void lcd_set_size(uint16_t width, uint16_t height)
{
  if (width != lcd_get_width() || height != lcd_get_height()) {
    mprintstr(PSTR("W: lcd_set_size(0x"));
    mprint_uint16(width, true);
    mprintstr(PSTR(", 0x"));
    mprint_uint16(height, true);
    mprintstrln(PSTR(")"));
  }
}

void lcd_set_address(bool a0)
{
  GPIO_WriteBit(MCODE_LCD_DC_PORT, MCODE_LCD_DC_PIN, a0 ? Bit_SET : Bit_RESET);
}
