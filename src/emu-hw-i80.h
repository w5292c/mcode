#ifndef MCODE_EMU_HW_I80_H
#define MCODE_EMU_HW_I80_H

#include "hw-i80.h"

void emu_hw_i80_init (void);
void emu_hw_i80_deinit (void);

void emu_hw_i80_set_read_callback (hw_i80_read_callback aCallback);

void emu_hw_i80_read (uint8_t cmd, uint8_t length);
void emu_hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *data);

void emu_hw_i80_reset (void);

#endif /* MC_EMU_CODE_I80_H */
