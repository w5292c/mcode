#include "hw-lcd-s95513.h"

#include "hw-i80.h"

#include <stdint.h>
#include <stdlib.h>

void hw_lcd_s95513_turn_on (void)
{
  /* exit_sleep_mode */
  hw_i80_write (UINT8_C(0x11), 0, NULL);
  /* set_display_on */
  hw_i80_write (UINT8_C(0x29), 0, NULL);
  /* set_pixel_format: 18bpp */
  uint8_t byte = 0x55;
  hw_i80_write (UINT8_C(0x3A), 1, &byte);
  byte = UINT8_C(0x02);
  hw_i80_write (UINT8_C(0x36), 1, &byte);
  hw_lcd_s95513_set_scroll_start (UINT16_C(0));
}

void hw_lcd_s95513_turn_off (void)
{
  /* set_display_off */
  hw_i80_write (UINT8_C(0x28), 0, NULL);
  /* enter_sleep_mode */
  hw_i80_write (UINT8_C(0x10), 0, NULL);
}

void hw_lcd_s95513_set_scroll_start (uint16_t start)
{
  uint8_t buffer[2];
  buffer[0] = (uint8_t)(start>>8);
  buffer[1] = (uint8_t)(start);
  hw_i80_write (UINT8_C (0x37), 2, buffer);
}
