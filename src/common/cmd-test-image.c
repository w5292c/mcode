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

#include "cmd-engine.h"

#include "hw-lcd.h"
#include "hw-uart.h"

#include <string.h>

static void cmd_test_image(void);
static void cmd_test_image_large(void);

void cmd_engine_images_help(void)
{
  hw_uart_write_string_P(PSTR("> timg - Load test image\r\n"));
  hw_uart_write_string_P(PSTR("> tlimg - Load large test image\r\n"));
}

bool cmd_engine_images_command(const char *command, bool *startCmd)
{
  if (!strcmp_P(command, PSTR("timg"))) {
    cmd_test_image();
    return true;
  } else if (!strcmp_P(command, PSTR("tlimg"))) {
    cmd_test_image_large();
    return true;
  }

  return false;
}

void cmd_test_image(void)
{
  const uint16_t width = lcd_get_width();
  const uint16_t height = lcd_get_height();

  const uint16_t TestColors[] = {
    UINT16_C(0x0000), UINT16_C(0x001F), UINT16_C(0x07E0), UINT16_C(0x07FF),
    UINT16_C(0xF800), UINT16_C(0xF81F), UINT16_C(0xFFE0), UINT16_C(0xFFFF),
  };
  const uint16_t n = sizeof (TestColors)/sizeof (*TestColors);
  const uint16_t lineHeight = height/n;
  const uint32_t pixelCount = (uint32_t)lineHeight*width;

  uint32_t i;
  for (i = 0; i < n; ++i) {
    const uint16_t startY = lineHeight*i;
    const uint16_t endY = startY + lineHeight - 1;
    lcd_set_window(0, width - 1, startY, endY);
    lcd_write_const_words(UINT8_C(0x2C), TestColors[i], pixelCount);
  }
}

void cmd_test_image_large(void)
{
  const uint16_t width = lcd_get_width();
  const uint16_t height = lcd_get_height();

  const uint16_t TestColors[] = {
    UINT16_C(0x0000), UINT16_C(0x001F), UINT16_C(0x07E0), UINT16_C(0x07FF),
    UINT16_C(0xF800), UINT16_C(0xF81F), UINT16_C(0xFFE0), UINT16_C(0xFFFF),
  };
  const uint16_t n = sizeof (TestColors)/sizeof (*TestColors);
  const uint16_t columnWidth = width/n;
  const uint32_t pixelCount = (uint32_t)columnWidth*height;

  uint32_t i;
  for (i = 0; i < n; ++i) {
    const uint16_t startX = columnWidth*i;
    const uint16_t endX = startX + columnWidth - 1;
    lcd_set_window(startX, endX, 0, height - 1);
    lcd_write_const_words(UINT8_C(0x2C), TestColors[i], pixelCount);
  }
}
