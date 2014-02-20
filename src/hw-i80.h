#ifndef MCODE_HW_I80_H
#define MCODE_HW_I80_H

#include <stdint.h>

typedef void (*hw_i80_read_callback) (int length, const unsigned char *data);

void hw_i80_init (void);
void hw_i80_deinit (void);

void hw_i80_set_read_callback (hw_i80_read_callback aCallback);

void hw_i80_read (uint8_t cmd, uint8_t length);
void hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *data);
void hw_i80_write_P (uint8_t cmd, uint8_t length, const uint8_t *data);

void hw_i80_write_words (uint8_t cmd, uint8_t length, const uint16_t *data);
void hw_i80_write_words_P (uint8_t cmd, uint8_t length, const uint16_t *data);

void hw_i80_write_const_short (uint8_t cmd, uint16_t constValue, uint8_t length);
void hw_i80_write_const (uint8_t cmd, uint16_t constValue, uint16_t length);
void hw_i80_write_const_long (uint8_t cmd, uint16_t constValue, uint32_t length);

/**
 * This function starts commans 'cmd' and then write 'dx'*'dy' words of data.
 *
 * The data is determined by bitmap in 'pData' like this:
 * - If the corresponding bit is 0: write offValue;
 * - If the corresponding bit is 1: write onValue;
 */
void hw_i80_write_bitmap (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue);
void hw_i80_write_bitmap_P (uint8_t cmd, uint16_t length, const uint8_t *pData, uint16_t offValue, uint16_t onValue);

void hw_i80_reset (void);

#endif /* MC_CODE_I80_H */
