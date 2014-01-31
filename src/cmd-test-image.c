#include "cmd-test-image.h"

#include "hw-i80.h"

#include <stdint.h>

/**
 * Create and load test image:
 * Top, left: 0x20, 0x20
 * Bottom, right: 0x80, 0x68
 * Dimensions: 0x60 (96) X 0x48 (72);
 */
void cmd_test_image (void)
{
  uint8_t buffer[24];
  /* set_column_address */
  buffer[0] = UINT8_C(0x00);
  buffer[1] = UINT8_C(0x00); /* start column */
  buffer[2] = UINT8_C(0x00);
  buffer[3] = UINT8_C(0x5F); /* end column */
  hw_i80_write (UINT8_C(0x2A), 4, buffer);
  /* set_page_address */
  buffer[0] = UINT8_C(0x00);
  buffer[1] = UINT8_C(0x00); /* start page */
  buffer[2] = UINT8_C(0x00);
  buffer[3] = UINT8_C(0x47); /* end page */
  hw_i80_write (UINT8_C(0x2B), 4, buffer);

  const uint32_t colors[12] = {
    /* format: 0xRRGGBB */
    0x000000U, 0x0000FFU, 0x00FF00U, 0x00FFFFU,
    0xFF0000U, 0xFF00FFU, 0xFFFF00U, 0xFFFFFFU,
    0x800000U, 0x800080U, 0x808000U, 0x808080U,
  };

  /* for evey line: do */
  uint8_t i, x, y, cmd;
  cmd = UINT8_C(0x2C);
  for (y = 0; y < 72; ++y)
  {
    for (x = 0; x < 96; x += 8)
    {
      /* prepare the buffer */
      const uint32_t color = colors[x/8];
      for (i = 0; i < 24; i += 3)
      {
        buffer[i + 0x00] = (uint8_t)(0xFFU&(color >> 16));
        buffer[i + 0x01] = (uint8_t)(0xFFU&(color >>  8));
        buffer[i + 0x02] = (uint8_t)(0xFFU&(color >>  0));
      }

      /* send the buffer to the LCD module */
      hw_i80_write (cmd, 24, buffer);
      cmd = UINT8_C(0x3C);
    }
  }
}

void cmd_test_image_large (void)
{
  uint8_t buffer[24];
  /* set_column_address */
  buffer[0] = UINT8_C(0x00);
  buffer[1] = UINT8_C(0x10); /* start column */
  buffer[2] = UINT8_C(0x01);
  buffer[3] = UINT8_C(0x2F); /* end column */
  hw_i80_write (UINT8_C(0x2A), 4, buffer);
  /* set_page_address */
  buffer[0] = UINT8_C(0x00);
  buffer[1] = UINT8_C(0x10); /* start page */
  buffer[2] = UINT8_C(0x01);
  buffer[3] = UINT8_C(0xcf); /* end page */
  hw_i80_write (UINT8_C(0x2B), 4, buffer);

  const uint32_t colors[12] = {
    /* format: 0xRRGGBB */
    0x000000U, 0x0000FFU, 0x00FF00U, 0x00FFFFU,
    0xFF0000U, 0xFF00FFU, 0xFFFF00U, 0xFFFFFFU,
    0x800000U, 0x800080U, 0x808000U, 0x808080U,
  };

  /* for evey line: do */
  uint8_t i, cmd;
  uint16_t x, y;
  cmd = UINT8_C(0x2C);
  for (y = 0; y < 224; ++y)
  {
    for (x = 0; x < 288; x += 8)
    {
      /* prepare the buffer */
      const uint32_t color = colors[(x/12)%12];
      for (i = 0; i < 24; i += 3)
      {
        buffer[i + 0x00] = (uint8_t)(0xFFU&(color >> 16));
        buffer[i + 0x01] = (uint8_t)(0xFFU&(color >>  8));
        buffer[i + 0x02] = (uint8_t)(0xFFU&(color >>  0));
      }

      /* send the buffer to the LCD module */
      hw_i80_write (cmd, 24, buffer);
      cmd = UINT8_C(0x3C);
    }
  }
}
