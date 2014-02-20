#ifndef MCODE_EMU_HW_I80_H
#define MCODE_EMU_HW_I80_H

#include "hw-i80.h"

void emu_hw_i80_init (void);
void emu_hw_i80_deinit (void);

void emu_hw_i80_set_read_callback (hw_i80_read_callback aCallback);

void emu_hw_i80_read (uint8_t cmd, uint8_t length);
void emu_hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *data);

void emu_hw_i80_reset (void);

void emu_hw_i80_write_words (uint8_t cmd, uint8_t length, const uint16_t *data);
void emu_hw_i80_write_words_P (uint8_t cmd, uint8_t length, const uint16_t *data);

void emu_hw_i80_write_const_short (uint8_t cmd, uint16_t constValue, uint8_t length);
void emu_hw_i80_write_const (uint8_t cmd, uint16_t constValue, uint16_t length);
void emu_hw_i80_write_const_long (uint8_t cmd, uint16_t constValue, uint32_t length);

void emu_hw_i80_write_bitmap (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue);
void emu_hw_i80_write_bitmap_P (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue);

#endif /* MC_EMU_CODE_I80_H */
