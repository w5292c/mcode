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
#include "hw-i80.h"
#include "hw-lcd.h"
#include "hw-uart.h"
#include "emu-common.h"
#include "customwidget.h"

#include <stdlib.h>

#define LCD_S95513_WR_RAM_START UINT8_C(0x2C)
#define LCD_S95513_WR_RAM_CONT UINT8_C(0x3C)
#define LCD_S95513_SET_COLUMN_ADDR UINT8_C(0x2A)
#define LCD_S95513_SET_PAGE_ADDR UINT8_C(0x2B)

static uint8_t TheBuffer[8];
static uint16_t ThePageEnd = 0;
static uint16_t TheNextPage = 0;
static uint16_t ThePageStart = 0;
static uint16_t TheColumnEnd = 0;
static uint8_t TheNormalFlag = 1;
static uint16_t TheNextColumn = 0;
static uint8_t TheBufferIndex = 0;
static uint16_t TheColumnStart = 0;
static uint8_t TheCurrentCommand = 0;

static AcCustomWidget *TheWidget = NULL;

static uint32_t emu_hw_lcd_s95513_to_color (quint32 data);
static void emu_hw_lcd_s95513_set_pixel (int x, int y, quint32 color);

/**
 * Commands
 */
static void emu_hw_lcd_s95513_handle_data_write_ram (uint16_t word);
static void emu_hw_lcd_s95513_handle_data_set_page_addr (uint8_t byte);
static void emu_hw_lcd_s95513_handle_data_set_column_addr (uint8_t byte);

static void emu_hw_lcd_s95513_handle_cmd (uint8_t cmd);
static void emu_hw_lcd_s95513_handle_data_byte (uint8_t byte);
static void emu_hw_lcd_s95513_handle_data_word (uint16_t word);

void lcd_set_scroll_start(uint16_t start)
{
  if (TheWidget) {
    TheWidget->setScrollPosition(start);
  } else {
    hw_uart_write_string_P(PSTR("lcd_set_scroll_start: no widget\r\n"));
  }
}

void hw_i80_init(void)
{
  if (!TheWidget)
  {
    TheWidget = new AcCustomWidget();
    TheWidget->show();
  }
}

void hw_i80_deinit(void)
{
  delete TheWidget;
  TheWidget = NULL;
}

void hw_i80_read(uint8_t cmd, uint8_t length)
{
}

void hw_i80_write(uint8_t cmd, uint8_t length, const uint8_t *data)
{
  uint8_t i;
  emu_hw_lcd_s95513_handle_cmd (cmd);
  for (i = 0; i < length; ++i)
  {
    emu_hw_lcd_s95513_handle_data_byte (*data++);
  }
}

void emu_hw_lcd_s95513_write_words (uint8_t cmd, uint8_t length, const uint16_t *data)
{
  uint8_t i;
  emu_hw_lcd_s95513_handle_cmd (cmd);
  for (i = 0; i < length; ++i)
  {
    emu_hw_lcd_s95513_handle_data_word (*data++);
  }
}

void hw_i80_reset (void)
{
  int x, y;
  for (x = 0; x < 320; ++x)
  {
    for (y = 0; y < 480; ++y)
    {
      TheWidget->setPixel (x, y, 0xff008080);
    }
  }
}

void emu_hw_lcd_s95513_set_pixel (int x, int y, quint32 color)
{
  TheWidget->setPixel (x, y, color);
}

void hw_i80_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  uint8_t bitMask;
  uint8_t currentByte;
  const uint8_t *const pDataEnd = pData + length;

  emu_hw_lcd_s95513_handle_cmd (cmd);
  /* write loop */
  currentByte = *pData++;
  for (bitMask = UINT8_C (0x01); ; )
  {
    emu_hw_lcd_s95513_handle_data_word ((currentByte & bitMask) ? onValue : offValue);
    bitMask = (bitMask << 1);
    if (!bitMask)
    {
      if (pData < pDataEnd)
      {
        bitMask = UINT8_C (0x01);
        currentByte = *pData++;
      }
      else
      {
        break;
      }
    }
  }
}

void hw_i80_write_bitmap_P(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  hw_i80_write_bitmap(cmd, length, pData, offValue, onValue);
}

uint32_t emu_hw_lcd_s95513_to_color (uint32_t data)
{
  const uint8_t red =   (0xffu&((0xf800U&data)>>8));
  const uint8_t green = (0xffu&((0x07E0U&data)>>3));
  const uint8_t blue =  (0xffu&((0x001FU&data)<<3));
  return 0xff000000u|(red << 16)|(green << 8)| blue;
}

void emu_hw_lcd_s95513_handle_cmd (uint8_t cmd)
{
  TheNormalFlag = 1;
  TheBufferIndex = 0;
  TheCurrentCommand = cmd;

  switch (cmd)
  {
  case LCD_S95513_WR_RAM_START:
    TheNextPage = ThePageStart;
    TheNextColumn = TheColumnStart;
    break;
  default:
    break;
  }
}

void emu_hw_lcd_s95513_handle_data_byte (uint8_t byte)
{
  switch (TheCurrentCommand)
  {
  case LCD_S95513_WR_RAM_CONT:
  case LCD_S95513_WR_RAM_START:
    emu_hw_lcd_s95513_handle_data_write_ram (byte);
  case LCD_S95513_SET_COLUMN_ADDR:
    emu_hw_lcd_s95513_handle_data_set_column_addr (byte);
    break;
  case LCD_S95513_SET_PAGE_ADDR:
    emu_hw_lcd_s95513_handle_data_set_page_addr (byte);
    break;
  default:
    hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_handle_data_byte (cmd: "));
    hw_uart_write_uint (TheCurrentCommand);
    hw_uart_write_string_P (PSTR (", byte: "));
    hw_uart_write_uint (byte);
    hw_uart_write_string_P (PSTR (")\r\n"));
    break;
  }
}

void emu_hw_lcd_s95513_handle_data_word (uint16_t word)
{
  switch (TheCurrentCommand)
  {
  case LCD_S95513_WR_RAM_CONT:
  case LCD_S95513_WR_RAM_START:
    emu_hw_lcd_s95513_handle_data_write_ram (word);
    break;
  case LCD_S95513_SET_COLUMN_ADDR:
    emu_hw_lcd_s95513_handle_data_set_column_addr ((uint8_t) word);
    break;
  case LCD_S95513_SET_PAGE_ADDR:
    emu_hw_lcd_s95513_handle_data_set_page_addr ((uint8_t) word);
    break;
  default:
    hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_handle_data_word (cmd: "));
    hw_uart_write_uint (TheCurrentCommand);
    hw_uart_write_string_P (PSTR (", word: "));
    hw_uart_write_uint (word);
    hw_uart_write_string_P (PSTR (")\r\n"));
    break;
  }
}

void emu_hw_lcd_s95513_handle_data_set_page_addr (uint8_t byte)
{
  if (TheNormalFlag)
  {
    TheBuffer[TheBufferIndex++] = byte;
    if (4 == TheBufferIndex)
    {
      ThePageStart = (TheBuffer[0] << 8) | TheBuffer[1];
      ThePageEnd = (TheBuffer[2] << 8) | TheBuffer[3];
      TheNormalFlag = 0;
    }
  }
}

void emu_hw_lcd_s95513_handle_data_set_column_addr (uint8_t byte)
{
  if (TheNormalFlag)
  {
    TheBuffer[TheBufferIndex++] = byte;
    if (4 == TheBufferIndex)
    {
      TheColumnStart = (TheBuffer[0] << 8) | TheBuffer[1];
      TheColumnEnd = (TheBuffer[2] << 8) | TheBuffer[3];
      TheNormalFlag = 0;
    }
  }
}

void emu_hw_lcd_s95513_handle_data_write_ram (uint16_t word)
{
  if (TheNormalFlag)
  {
    /* got next color sample, put it to the surface */
    const uint32_t color = emu_hw_lcd_s95513_to_color (word);
    emu_hw_lcd_s95513_set_pixel (TheNextColumn, TheNextPage, color);

    TheNextColumn++;
    if (TheNextColumn > TheColumnEnd)
    {
      TheNextColumn = TheColumnStart;
      TheNextPage++;
      if (TheNextPage > ThePageEnd)
      {
        TheNormalFlag = 0;
      }
    }
  }
}

void hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
}

uint16_t lcd_get_width(void)
{
  return 320;
}

uint16_t lcd_get_height(void)
{
  return 480;
}

void lcd_write_const_words(uint8_t cmd, uint16_t word, uint32_t count)
{
  hw_i80_write_const_long (cmd, word, count);
}

void lcd_write_bitmap(uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  hw_i80_write_bitmap(cmd, length, pData, offValue, onValue);
}

void lcd_turn(bool on)
{
}

void lcd_reset(void)
{
}

void lcd_set_bl(bool on)
{
}

uint32_t lcd_read_id(void)
{
  return 0;
}

void lcd_set_window(uint16_t colStart, uint16_t colEnd, uint16_t rowStart, uint16_t rowEnd)
{
  uint8_t buffer[4];
  /* set_column_address */
  buffer[0] = UINT8_C(colStart>>8);
  buffer[1] = UINT8_C(colStart);
  buffer[2] = UINT8_C(colEnd>>8);
  buffer[3] = UINT8_C(colEnd);
  hw_i80_write(UINT8_C(0x2A), 4, buffer);
  /* set_page_address */
  buffer[0] = UINT8_C(rowStart>>8);
  buffer[1] = UINT8_C(rowStart);
  buffer[2] = UINT8_C(rowEnd>>8);
  buffer[3] = UINT8_C(rowEnd);
  hw_i80_write(UINT8_C(0x2B), 4, buffer);
}

void hw_i80_write_const_short(uint8_t cmd, uint16_t constValue, uint8_t length)
{
  hw_i80_write_const_long(cmd, constValue, length);
}

void hw_i80_write_const(uint8_t cmd, uint16_t constValue, uint16_t length)
{
  hw_i80_write_const_long(cmd, constValue, length);
}

void hw_i80_write_const_long(uint8_t cmd, uint16_t constValue, uint32_t length)
{
  uint32_t i;
  emu_hw_lcd_s95513_handle_cmd(cmd);
  for (i = 0; i < length; ++i) {
    emu_hw_lcd_s95513_handle_data_word(constValue);
  }
}
