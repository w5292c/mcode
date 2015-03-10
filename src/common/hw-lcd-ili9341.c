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
#include "console.h"

static uint8_t lcd_read_register(uint8_t addr, uint8_t param);

uint16_t lcd_get_width(void)
{
  return UINT16_C(240);
}

uint16_t lcd_get_height(void)
{
  return UINT16_C(320);
}

void lcd_turn(bool on)
{
  lcd_command(on ? UINT8_C(0x29) : UINT8_C(0x28));
}

uint32_t lcd_read_id(void)
{
  int i;
  uint8_t id[3] = {0};
  for (i = 0; i < 3; ++i) {
    id[i] = lcd_read_register(UINT8_C(0xD3), i + 1);
  }

  const uint32_t idValue = ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8) | id[2];
  return idValue;
}

uint8_t lcd_read_register(uint8_t addr, uint8_t param)
{
  lcd_command(UINT8_C(0xD9), UINT8_C(0x10) + param);
  return lcd_read_byte(addr);
}

void lcd_device_init(void)
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
