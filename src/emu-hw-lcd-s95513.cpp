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

static hw_i80_read_callback TheReadCallback = NULL;

static uint32_t emu_hw_lcd_s95513_to_color (quint32 data);
static void emu_hw_lcd_s95513_set_pixel (int x, int y, quint32 color);

/**
 * Commands
 */
static void emu_hw_lcd_s95513_handle_write_start (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_write_continue (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_set_page_addr (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_set_column_addr (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_write_const_start (uint32_t length, uint32_t data);
static void emu_hw_lcd_s95513_handle_write_const_continue (uint32_t length, uint32_t data);
static void emu_hw_lcd_s95513_handle_write_double_start (uint32_t length, const uint8_t *data);
static void emu_hw_lcd_s95513_handle_write_double_continue (uint32_t length, const uint8_t *data);

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
  switch (cmd)
  {
  case LCD_S95513_WR_RAM_START:
    emu_hw_lcd_s95513_handle_write_start (length, data);
    break;
  case LCD_S95513_WR_RAM_CONT:
    emu_hw_lcd_s95513_handle_write_continue (length, data);
    break;
  case LCD_S95513_SET_COLUMN_ADDR:
    emu_hw_lcd_s95513_handle_set_column_addr (length, data);
    break;
  case LCD_S95513_SET_PAGE_ADDR:
    emu_hw_lcd_s95513_handle_set_page_addr (length, data);
    break;
  default:
    hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_write (cmd: "));
    hw_uart_write_uint (cmd);
    hw_uart_write_string_P (PSTR (", data length: "));
    hw_uart_write_uint (length);
    hw_uart_write_string_P (PSTR (")\r\n"));
    break;
  }
}

void emu_hw_lcd_s95513_write_double (uint8_t cmd, uint8_t length, const uint8_t *data)
{
  switch (cmd)
  {
  case LCD_S95513_WR_RAM_START:
    emu_hw_lcd_s95513_handle_write_double_start (length, data);
    break;
  case LCD_S95513_WR_RAM_CONT:
    emu_hw_lcd_s95513_handle_write_double_continue (length, data);
    break;
  default:
    hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_write_double (cmd: "));
    hw_uart_write_uint (cmd);
    hw_uart_write_string_P (PSTR (", data length: "));
    hw_uart_write_uint (length);
    hw_uart_write_string_P (PSTR (")\r\n"));
    break;
  }
}

void emu_hw_lcd_s95513_write_double_P (uint8_t cmd, uint8_t length, const uint8_t *data)
{
  emu_hw_lcd_s95513_write_double (cmd, length, data);
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
  switch (cmd)
  {
  case LCD_S95513_WR_RAM_START:
    emu_hw_lcd_s95513_handle_write_const_start (length, constValue);
    break;
  case LCD_S95513_WR_RAM_CONT:
    emu_hw_lcd_s95513_handle_write_const_continue (length, constValue);
    break;
  default:
    hw_uart_write_string_P (PSTR ("EMU: emu_hw_lcd_s95513_write_const_long (cmd: "));
    hw_uart_write_uint (cmd);
    hw_uart_write_string_P (PSTR (", data length: "));
    hw_uart_write_uint (length);
    hw_uart_write_string_P (PSTR (")\r\n"));
    break;
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

static uint32_t TheCache = 0;
static uint16_t ThePageEnd = 0;
static uint16_t TheNextPage = 0;
static uint16_t ThePageStart = 0;
static uint16_t TheColumnEnd = 0;
static uint16_t TheColumnStart = 0;
static uint16_t TheNextColumn = 0;
static uint32_t TheWriteIndex = 0;

static int8_t TheState = 0;
static void emu_hw_lcd_s95513_put_byte (uint8_t data)
{
  if (TheState < 0)
  {
    /* ignore-state */
    return;
  }

  TheCache = (TheCache << 8);
  TheCache = (TheCache | data);
  ++TheWriteIndex;
  if (0 == (TheWriteIndex%2))
  {
    /* got next color sample, put it to the surface */
    const uint32_t color = emu_hw_lcd_s95513_to_color (TheCache);
    emu_hw_lcd_s95513_set_pixel (TheNextColumn, TheNextPage, color);

    TheNextColumn++;
    if (TheNextColumn > TheColumnEnd)
    {
      TheNextColumn = TheColumnStart;
      TheNextPage++;
      if (TheNextPage > ThePageEnd)
      {
        TheState = -1;
      }
    }

    /* clean-up before the next sample */
    TheCache = 0;
  }
}

void emu_hw_lcd_s95513_handle_write_start (uint32_t length, const uint8_t *data)
{
  TheState = 0;
  TheCache = 0;
  TheWriteIndex = 0;
  TheNextPage = ThePageStart;
  TheNextColumn = TheColumnStart;

  while (length--)
  {
    emu_hw_lcd_s95513_put_byte (*data++);
  }
}

void emu_hw_lcd_s95513_handle_write_continue (uint32_t length, const uint8_t *data)
{
  while (length--)
  {
    emu_hw_lcd_s95513_put_byte (*data++);
  }
}

void emu_hw_lcd_s95513_handle_write_const_start (uint32_t length, uint32_t data)
{
  TheState = 0;
  TheCache = 0;
  TheWriteIndex = 0;
  TheNextPage = ThePageStart;
  TheNextColumn = TheColumnStart;

  const uint8_t byte0 = (uint8_t)(data >> 8);
  const uint8_t byte1 = (uint8_t)data;
  while (length--)
  {
    emu_hw_lcd_s95513_put_byte (byte0);
    emu_hw_lcd_s95513_put_byte (byte1);
  }
}

void emu_hw_lcd_s95513_handle_write_const_continue (uint32_t length, uint32_t data)
{
  const uint8_t byte0 = (uint8_t)(data >> 8);
  const uint8_t byte1 = (uint8_t)data;
  while (length--)
  {
    emu_hw_lcd_s95513_put_byte (byte0);
    emu_hw_lcd_s95513_put_byte (byte1);
  }
}

void emu_hw_lcd_s95513_handle_write_double_start (uint32_t length, const uint8_t *data)
{
  TheState = 0;
  TheCache = 0;
  TheWriteIndex = 0;
  TheNextPage = ThePageStart;
  TheNextColumn = TheColumnStart;

  emu_hw_lcd_s95513_handle_write_double_continue (length, data);
}

void emu_hw_lcd_s95513_handle_write_double_continue (uint32_t length, const uint8_t *data)
{
  int i;
  for (i = 0; i < length; i += 2)
  {
    const uint8_t data0 = *data++;
    const uint8_t data1 = *data++;
    emu_hw_lcd_s95513_put_byte (data0);
    emu_hw_lcd_s95513_put_byte (data1);
  }
}

void emu_hw_lcd_s95513_handle_set_page_addr (uint32_t length, const uint8_t *data)
{
  /* do not support general case for now, to be implemented */
  /*g_assert (4 == length);*/

  ThePageStart = ((*data++)<<8) | (*data++);
  ThePageEnd = ((*data++)<<8) | (*data++);
}

void emu_hw_lcd_s95513_handle_set_column_addr (uint32_t length, const uint8_t *data)
{
  /* do not support general case for now, to be implemented */
  /*g_assert (4 == length);*/

  TheColumnStart = ((*data++)<<8) | (*data++);
  TheColumnEnd = ((*data++)<<8) | (*data++);
}

uint32_t emu_hw_lcd_s95513_to_color (uint32_t data)
{
  const uint8_t red =   (0xffu&((0xf800U&data)>>8));
  const uint8_t green = (0xffu&((0x07E0U&data)>>3));
  const uint8_t blue =  (0xffu&((0x001FU&data)<<3));
  return 0xff000000u|(red << 16)|(green << 8)| blue;
}
