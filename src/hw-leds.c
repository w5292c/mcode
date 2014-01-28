#include "hw-leds.h"

#include "hw-uart.h"
#include "mcode-config.h"

#ifdef __AVR__
#include <avr/io.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

#ifdef MCODE_EMULATE_LED
static uint8_t TheLedStates = 0;
#endif /* MCODE_EMULATE_LED */

#ifdef __AVR__
inline static uint8_t
mcode_hw_leds_get_led_bit (int index) { return (index == 0) ? (1U << PB2) : (1U << PB3); }
#endif /* __AVR__ */

/*
 * Test code that manages 2 test LEDs connected to PB2, PB3
 */
void mcode_hw_leds_init (void)
{
#ifdef __AVR__
  /* configure PB2, PB3 as outputs */
  DDRB |= ((1U << DDB2)|(1U << DDB3));
  /* turn both LEDs OFF */
  PORTB &= ~((1U << PB3)|(1U << PB2));
#endif /* __AVR__ */

#ifdef MCODE_EMULATE_LED
  TheLedStates = 0;
#endif /* MCODE_EMULATE_LED */
}

void mcode_hw_leds_deinit (void)
{
}

void mcode_hw_leds_set (int index, int on)
{
#ifndef MCODE_EMULATE_LED

#ifdef __AVR__
  const uint8_t ledBit = mcode_hw_leds_get_led_bit (index);

  if (on) {
    PORTB |= ledBit;
  } else {
    PORTB &= ~ledBit;
  }
#endif /* __AVR__ */

#else /* MCODE_EMULATE_LED */
  hw_uart_write_string_P (PSTR("Setting LED"));
  hw_uart_write_uint (index);
  hw_uart_write_string_P (PSTR(": "));
  hw_uart_write_string_P (on ? PSTR("ON"): PSTR("OFF"));
  hw_uart_write_string_P (PSTR("\r\n"));

  if (index >= 0 && index < 8)
  {
    if (on)
    {
      TheLedStates |= (1U << index);
    }
    else
    {
      TheLedStates &= ~(1U << index);
    }
  }
#endif /* MCODE_EMULATE_LED */
}

int mcode_hw_leds_get (int index)
{
#ifndef MCODE_EMULATE_LED
  const uint8_t ledBit = mcode_hw_leds_get_led_bit (index);
  return (PORTB & ledBit) ? 1 : 0;
#else /* MCODE_EMULATE_LED */
  return (index >=0 && index < 8) ? (TheLedStates & (1U << index)) : 0;
#endif /* MCODE_EMULATE_LED */
}
