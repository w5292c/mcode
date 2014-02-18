#ifndef MCODE_EMU_COMMON
#define MCODE_EMU_COMMON

#define PROGMEM
#define PSTR(value) (value)
#define strcmp_P strcmp
#define strncmp_P strncmp
#define strlen_P strlen

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

inline static void sei (void) {}
inline static uint8_t pgm_read_byte (const uint8_t *pAddress) { return *pAddress; }
inline static uint16_t pgm_read_word (const uint16_t *pAddress) { return *pAddress; }

#endif /* MCODE_EMU_COMMON */
