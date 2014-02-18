#ifndef MCODE_HW_LEDS_H
#define MCODE_HW_LEDS_H

#ifdef __cplusplus
extern "C" {
#endif

void mcode_hw_leds_init (void);
void mcode_hw_leds_deinit (void);

void mcode_hw_leds_set (int index, int on);
int mcode_hw_leds_get (int index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_HW_LEDS_H */
