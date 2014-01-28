#include "emu-hw-i80-lcd1.h"

#include "hw-uart.h"
#include "mcode-config.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

#ifndef MCODE_WMU_HW_I80_LCD1_H
#error "This file can only be used in MCODE_WMU_HW_I80_LCD1_H mode"
#endif /* MCODE_WMU_HW_I80_LCD1_H */

typedef struct
{
  int m_length;
  const unsigned char *m_pData;
} I80LcdRegsData;

static hw_i80_read_callback TheReadCallback = NULL;
static hw_i80_write_callback TheWriteCallback = NULL;

static void hw_lcd1_get_regs_data(I80LcdRegsData *pData);

void emu_lcd1_hw_i80_init (void)
{
}

void emu_lcd1_hw_i80_deinit (void)
{
}

void emu_lcd1_hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
  TheReadCallback = aCallback;
}

void emu_lcd1_hw_i80_set_write_callback (hw_i80_write_callback aCallback)
{
  TheWriteCallback = aCallback;
}

void emu_lcd1_hw_i80_read (unsigned char cmd, int length)
{
  hw_uart_write_string_P (PSTR("READ request, command: "));
  hw_uart_write_uint (cmd);
  hw_uart_write_string_P (PSTR("\r\n"));

  if (TheReadCallback)
  {
    unsigned char buffer[64] = {
      /* test data, const for now */
      /**@todo update */
      0x12, 0x23, 0x34, 0x45,
      0x56, 0x67, 0x78, 0x89,
      0x9A, 0xAB, 0xBC, 0xCD,
      0xFF, 0xFF, 0xFF, 0xFF,
    };
    I80LcdRegsData data = {0, NULL};
    hw_lcd1_get_regs_data (&data);

    (*TheReadCallback) (length, buffer);
  }
}

void emu_lcd1_hw_i80_write (unsigned char cmd, int length, const unsigned char *data)
{
  int i;
  hw_uart_write_string_P (PSTR("WRITE request, CMD: ["));
  hw_uart_write_uint (cmd);
  hw_uart_write_string_P (PSTR("], length: "));
  hw_uart_write_uint (length);
  hw_uart_write_string_P (PSTR(", data:\r\n"));
  for (i = 0; i < length; i++)
  {
    hw_uart_write_uint (data[i]);
    if (i != (length -1))
    {
      hw_uart_write_string_P (PSTR(" "));
    }
  }
  hw_uart_write_string_P (PSTR("\r\n"));

  if (TheWriteCallback)
  {
    (*TheWriteCallback) (length);
  }
}

void emu_lcd1_hw_i80_reset (void)
{
  hw_uart_write_string_P (PSTR("LCD1: reset module\r\n"));
}

void hw_lcd1_get_regs_data(I80LcdRegsData *pData)
{
}
