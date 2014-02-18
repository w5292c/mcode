#ifndef MCODE_HW_LCD_S95513_H
#define MCODE_HW_LCD_S95513_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void hw_lcd_s95513_turn_on (void);
void hw_lcd_s95513_turn_off (void);
void hw_lcd_s95513_set_scroll_start (uint16_t start);

#define LCD_S95513_WR_RAM_START UINT8_C(0x2C)
#define LCD_S95513_WR_RAM_CONT UINT8_C(0x3C)
#define LCD_S95513_SET_COLUMN_ADDR UINT8_C(0x2A)
#define LCD_S95513_SET_PAGE_ADDR UINT8_C(0x2B)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_HW_LCD_S95513_H */
