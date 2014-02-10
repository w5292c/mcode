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
  const uint8_t format = 0x55;
  hw_i80_write (UINT8_C(0x3A), 1, &format);
}

void hw_lcd_s95513_turn_off (void)
{
  /* set_display_off */
  hw_i80_write (UINT8_C(0x28), 0, NULL);
  /* enter_sleep_mode */
  hw_i80_write (UINT8_C(0x10), 0, NULL);
}
