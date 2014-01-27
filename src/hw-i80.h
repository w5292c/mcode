#ifndef MCODE_HW_I80_H
#define MCODE_HW_I80_H

typedef void (*hw_i80_write_callback) (int length);
typedef void (*hw_i80_read_callback) (int length, const unsigned char *data);

void hw_i80_init (void);
void hw_i80_deinit (void);

void hw_i80_set_read_callback (hw_i80_read_callback aCallback);
void hw_i80_set_write_callback (hw_i80_write_callback aCallback);

void hw_i80_read (unsigned char cmd, int length);
void hw_i80_write (unsigned char cmd, int length, const unsigned char *data);

void hw_i80_reset (void);

#endif /* MC_CODE_I80_H */
