#ifndef MCODE_EMU_HW_LCD_S95513_H
#define MCODE_EMU_HW_LCD_S95513_H

#include "hw-i80.h"
#include "hw-lcd-s95513.h"

#ifdef __cplusplus
extern "C" {
#endif

void emu_hw_lcd_s95513_init (void);
void emu_hw_lcd_s95513_deinit (void);

void emu_hw_lcd_s95513_turn_on (void);
void emu_hw_lcd_s95513_turn_off (void);
void emu_hw_lcd_s95513_set_scroll_start (uint16_t start);

void emu_hw_lcd_s95513_reset (void);

void emu_hw_lcd_s95513_set_read_callback (hw_i80_read_callback aCallback);
void emu_hw_lcd_s95513_read (uint8_t cmd, uint8_t length);
void emu_hw_lcd_s95513_write (uint8_t cmd, uint8_t length, const uint8_t *data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_EMU_HW_LCD_S95513_H */
