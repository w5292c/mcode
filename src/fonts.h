#ifndef MCODE_FONTS_H
#define MCODE_FONTS_H

#include <stdint.h>

uint8_t mcode_fonts_get_bitmap (uint8_t code, uint8_t line);
const uint8_t *mcode_fonts_get_char_bitmap (uint8_t code);

#endif /* MCODE_FONTS_H */
