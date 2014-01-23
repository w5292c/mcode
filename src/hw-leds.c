#include "hw-leds.h"

#include <avr/io.h>

inline static uint8_t
mcode_hw_leds_get_led_bit (int index) { return (index == 0) ? (1U << PB2) : (1U << PB3); }

/*
 * Test code that manages 2 test LEDs connected to PB2, PB3
 */
void mcode_hw_leds_init (void)
{
  /* configure PB2, PB3 as outputs */
  DDRB |= ((1U << DDB2)|(1U << DDB3));
  /* set PB2 and reset PB3 */
  PORTB |= (1U << PB2);
  PORTB &= ~(1U << PB3);
}

void mcode_hw_leds_deinit (void)
{
}

void mcode_hw_leds_set (int index, int on)
{
  const uint8_t ledBit = mcode_hw_leds_get_led_bit (index);

  if (on) {
    PORTB |= ledBit;
  } else {
    PORTB &= ~ledBit;
  }
}

int mcode_hw_leds_get (int index)
{
  const uint8_t ledBit = mcode_hw_leds_get_led_bit (index);
  return (PORTB & ledBit) ? 1 : 0;
}
