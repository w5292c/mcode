#include "hw-i80.h"

#include "mcode-config.h"

#ifndef MCODE_EMULATE_I80
void hw_i80_init (void)
{
}

void hw_i80_deinit (void)
{
}

void hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
}

void hw_i80_set_write_callback (hw_i80_write_callback aCallback)
{
}

void hw_i80_write (unsigned char cmd, int length, const unsigned char *data)
{
}

void hw_i80_read (unsigned char cmd, int length)
{
}

#else /* MCODE_EMULATE_I80 */
void hw_i80_init (void) { emu_hw_i80_init (); }
void hw_i80_deinit (void) { emu_hw_i80_deinit (); }
void hw_i80_set_read_callback (hw_i80_read_callback aCallback) { emu_hw_i80_set_read_callback (aCallback); }
void hw_i80_set_write_callback (hw_i80_write_callback aCallback) { emu_hw_i80_set_write_callback (aCallback); }
void hw_i80_read (unsigned char cmd, int length) { emu_hw_i80_read (cmd, length); }
void hw_i80_write (unsigned char cmd, int length, const unsigned char *data) { emu_hw_i80_write (cmd, length, data); }

#endif /* MCODE_EMULATE_I80 */
