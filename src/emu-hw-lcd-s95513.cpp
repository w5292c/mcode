#include "emu-hw-lcd-s95513.h"

#include "hw-uart.h"
#include "customwidget.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"

#include <stdio.h>
#include <stdlib.h>
#endif /* __AVR__ */

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
static hw_i80_read_callback TheReadCallback = NULL;

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

void emu_hw_lcd_s95513_turn_on (void)
{
  hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_turn_on\r\n"));
}

void emu_hw_lcd_s95513_turn_off (void)
{
  hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_turn_off\r\n"));
}

void emu_hw_lcd_s95513_set_scroll_start (uint16_t start)
{
  hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_set_scroll_start ("));
  hw_uart_write_uint (start);
  hw_uart_write_string_P (PSTR (")\r\n"));
}

static AcCustomWidget *TheWidget = NULL;
void emu_hw_lcd_s95513_init (void)
{
  if (!TheWidget)
  {
    TheWidget = new AcCustomWidget();
    TheWidget->show();
  }
}

void emu_hw_lcd_s95513_deinit (void)
{
  delete TheWidget;
  TheWidget = NULL;
}

void emu_hw_lcd_s95513_set_read_callback (hw_i80_read_callback aCallback)
{
  TheReadCallback = aCallback;
}

void emu_hw_lcd_s95513_read (uint8_t cmd, uint8_t length)
{
  if (TheReadCallback)
  {
    /**@todo Implement reading, return empty data for now */
    (*TheReadCallback) (0, NULL);
  }
}

void emu_hw_lcd_s95513_write (uint8_t cmd, uint8_t length, const uint8_t *data)
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

void emu_hw_lcd_s95513_write_words_P (uint8_t cmd, uint8_t length, const uint16_t *data)
{
  emu_hw_lcd_s95513_write_words (cmd, length, data);
}

void emu_hw_lcd_s95513_write_const_short (uint8_t cmd, uint16_t constValue, uint8_t length)
{
  emu_hw_lcd_s95513_write_const_long (cmd, constValue, length);
}

void emu_hw_lcd_s95513_write_const (uint8_t cmd, uint16_t constValue, uint16_t length)
{
  emu_hw_lcd_s95513_write_const_long (cmd, constValue, length);
}

void emu_hw_lcd_s95513_write_const_long (uint8_t cmd, uint16_t constValue, uint32_t length)
{
  uint32_t i;

  emu_hw_lcd_s95513_handle_cmd (cmd);
  for (i = 0; i < length; ++i)
  {
    emu_hw_lcd_s95513_handle_data_word (constValue);
  }
}

void emu_hw_lcd_s95513_reset (void)
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

void emu_hw_lcd_s95513_write_bitmap (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  uint8_t bitMask;
  uint8_t currentByte;
  const uint8_t *const pDataEnd = pData + length;

  emu_hw_lcd_s95513_handle_cmd (cmd);
  /* write loop */
  currentByte = *pData++;
  for (bitMask = UINT8_C (0x80), currentByte = *pData++; ; )
  {
    emu_hw_lcd_s95513_handle_data_word ((currentByte & bitMask) ? onValue : offValue);
    if (!bitMask)
    {
      if ((++pData) < pDataEnd)
      {
        bitMask = UINT8_C (0x80);
        currentByte = *pData;
      }
      else
      {
        break;
      }
    }
  }
}

void emu_hw_lcd_s95513_write_bitmap_P (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue)
{
  emu_hw_lcd_s95513_write_bitmap (cmd, length, pData, offValue, onValue);
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
