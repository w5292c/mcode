#include "emu-hw-i80-lcd1.h"

#include "mcode-config.h"

#include <stdio.h>
#include <stdlib.h>

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
  printf ("READ request, command: 0x%X\n", cmd);

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
  printf ("WRITE request, CMD: [0x%2.2x], length %d, data:\n>> ", cmd, length);
  for (i = 0; i < length; i++)
  {
    printf ("%2.2X%s", data[i], (i == (length -1)) ? "" : " ");
  }
  printf ("\n");

  if (TheWriteCallback)
  {
    (*TheWriteCallback) (length);
  }
}

void hw_lcd1_get_regs_data(I80LcdRegsData *pData)
{
}
