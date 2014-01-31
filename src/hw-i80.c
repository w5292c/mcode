#include "hw-i80.h"

#include "mcode-config.h"

#ifndef MCODE_EMULATE_I80
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/cpufunc.h>
#include <util/delay_basic.h>

/* HW configuration:
|---------------------------------------------------------------------------|
|  # | LCD Pin | UC Pin | Comment                                           |
|----|---------|--------|---------------------------------------------------|
|  1 |      D0 |    PA0 |                                                   |
|  2 |      D1 |    PA1 |                                                   |
|  3 |      D2 |    PA2 |                                                   |
|  4 |      D3 |    PA3 |                                                   |
|  5 |      D4 |    PA4 |                                                   |
|  6 |      D5 |    PA5 |                                                   |
|  7 |      D6 |    PA6 |                                                   |
|  8 |      D7 |    PA7 |                                                   |
|----|---------|--------|---------------------------------------------------|
|  9 |      CS |    PC0 | TBC; Prev pin: PB0                                |
| 10 |      WR |    PC1 | TBC; Prev pin: PB1                                |
| 11 |      RD |    PC6 | TBC; Prev pin: PB2                                |
| 12 |   A0/RS |    PC7 | TBC; Prev pin: PB3                                |
| 13 |   RESET |    PD7 | TBC; Prev pin: PB4                                |
|---------------------------------------------------------------------------|
*/
/* Available pins: PC0, PC1, PC6, PC7, PD7 */

#define HW_I80_READ_BUFFER_LENGTH (16)
static unsigned char TheReadBuffer[HW_I80_READ_BUFFER_LENGTH];
static hw_i80_read_callback TheReadCallback = NULL;

inline static void hw_i80_activate_cs (void) { PORTC &= ~(1 << PC0); }
inline static void hw_i80_deactivate_cs (void) { PORTC |= (1 << PC0); }

inline static void hw_i80_activate_wr (void) { PORTC &= ~(1 << PC1); }
inline static void hw_i80_deactivate_wr (void) { PORTC |= (1 << PC1); }

inline static void hw_i80_activate_rd (void) { PORTC &= ~(1 << PC6); }
inline static void hw_i80_deactivate_rd (void) { PORTC |= (1 << PC6); }

inline static void hw_i80_activate_rd_wr (void) { PORTC &= ~((1 << PC1)|(1 << PC6)); }
inline static void hw_i80_deactivate_rd_wr (void) { PORTC |= ((1 << PC1)|(1 << PC6)); }

inline static void hw_i80_activate_cmd (void) { PORTC &= ~(1 << PC7); } /* 0 */
inline static void hw_i80_activate_data (void) { PORTC |= (1 << PC7); } /* 1 */

inline static void hw_i80_activate_reset (void) { PORTD &= ~(1 << PD7); }
inline static void hw_i80_deactivate_reset (void) { PORTD |= (1 << PD7); }

inline static void hw_i80_set_data_port_in (void) { DDRA = UINT8_C(0x00); PORTA = UINT8_C(0xFF); }
inline static void hw_i80_set_data_port_out (void) { PORTA = UINT8_C(0xFF); DDRA = UINT8_C(0xFF); }

inline static uint8_t hw_i80_read_data (void) { return PINA; }
inline static void hw_i80_write_data (uint8_t data) { PORTA = data; }

inline static void hw_i80_read_write_delay (void) { _NOP (); }

static void hw_i80_setup_ports (void) {
  /* Configure C and D-ports, outputs: CS, WR, RD, RS, RESET */
  DDRD = (1U << DDD7); DDRC = ((1U << DDC0)|(1U << DDC1)|(1U << DDC6)|(1U << DDC7));
  /* Reset all C and D-port outputs to inactive state (1) */
  PORTD = (1U << PD7); PORTC = ((1U << PC0)|(1U << PC1)|(1U << PC6)|(1U << PC7)); _NOP ();
}

void hw_i80_init (void)
{
  /* setup pins */
  hw_i80_setup_ports ();
  /* make all the A-port pins as inputs, disable pull-up resistors */
  hw_i80_set_data_port_in ();
}

void hw_i80_deinit (void)
{
  /* reset pin config to default */
  hw_i80_setup_ports ();
  /* make all the A-port pins as inputs, disable pull-up resistors */
  hw_i80_set_data_port_in ();
}

void hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
  TheReadCallback = aCallback;
}

void hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *pData)
{
  /* activate CS */
  hw_i80_activate_cs ();

  /* activate D/CX */
  hw_i80_activate_cmd ();
  /* configure PORTA as outputs */
  hw_i80_set_data_port_out ();
  /* send the command ID to the port A */
  hw_i80_write_data (cmd);
  /* activate WR */
  hw_i80_activate_wr ();
  /* some delay, todo: check if this is required */
  hw_i80_read_write_delay ();
  /* deactivate WR */
  hw_i80_deactivate_wr ();
  /* deactivate D/CX */
  hw_i80_activate_data ();
  /* some delay, todo: check if this is required */
  hw_i80_read_write_delay ();

  /* write cycle */
  int i;
  for (i = 0; i < length; ++i)
  {
    const uint8_t data = pData[i];
    hw_i80_write_data (data);
    hw_i80_activate_wr ();
    /* some delay, todo: check if this is required */
    hw_i80_read_write_delay ();
    hw_i80_deactivate_wr ();
    /* some delay, todo: check if this is required */
    hw_i80_read_write_delay ();
  }

  /* clean-up */
  /* configure PORTA as inputs */
  hw_i80_set_data_port_in ();

  /* deactivate CS */
  hw_i80_deactivate_cs ();
}

void hw_i80_read (uint8_t cmd, uint8_t length)
{
  if (length > HW_I80_READ_BUFFER_LENGTH)
  {
    /* we cannot read more than the length of the buffer we have */
    length = HW_I80_READ_BUFFER_LENGTH;
  }

  /* activate CS */
  hw_i80_activate_cs ();

  /* activate D/CX */
  hw_i80_activate_cmd ();
  /* configure PORTA as outputs */
  hw_i80_set_data_port_out ();
  /* send the command ID to the port A */
  hw_i80_write_data (cmd);
  /* activate WR */
  hw_i80_activate_wr ();
  /* some delay, todo: check if this is required */
  hw_i80_read_write_delay ();
  /* deactivate WR */
  hw_i80_deactivate_wr ();
  /* some delay, todo: check if this is required */
  hw_i80_read_write_delay ();
  /* deactivate D/CX */
  hw_i80_activate_data ();
  /* confugure PORTA as input, do not enable pull-up resistors */
  hw_i80_set_data_port_in ();
  /* some delay, todo: check if this is required */
  hw_i80_read_write_delay ();
#if 1 /* incorrect read request, todo: check if this is required */
  /* activate WRX and RDX pins */
  hw_i80_activate_rd_wr ();
  /* some delay, todo: check if this is required */
  hw_i80_read_write_delay ();
  /* deactivate WRX and RDX */
  hw_i80_deactivate_rd_wr ();
  /* some delay, todo: check if this is required */
  hw_i80_read_write_delay ();
#endif /* end: incorrect read request, todo: check if this is required */
  /* Read cycle */
  memset (TheReadBuffer, 0, HW_I80_READ_BUFFER_LENGTH);
  int i;
  for (i = 0; i < length; ++i)
  {
    /* correct reads, activate RDX */
    hw_i80_activate_rd ();
    /* some delay, todo: check if this is required */
    hw_i80_read_write_delay ();
    /* Now, read the PORTA */
    unsigned char data = hw_i80_read_data ();
    TheReadBuffer[i] = data;
    /* and deactivate RDX */
    hw_i80_deactivate_rd ();
    /* some delay, todo: check if this is required */
    hw_i80_read_write_delay ();
  }

  /* now, move to the idle state */
  /* deactivate CS */
  hw_i80_deactivate_cs ();

  /* and finally, notify the client */
  if (TheReadCallback)
  {
    (*TheReadCallback) (length, TheReadBuffer);
  }
}

void hw_i80_reset (void)
{
  hw_i80_activate_reset ();
  _delay_loop_2 (0xFFFFU);
  hw_i80_deactivate_reset ();
  _delay_loop_2 (0xFFFFU);
}

#else /* MCODE_EMULATE_I80 */
#include "emu-hw-i80.h"

void hw_i80_init (void) { emu_hw_i80_init (); }
void hw_i80_deinit (void) { emu_hw_i80_deinit (); }
void hw_i80_set_read_callback (hw_i80_read_callback aCallback) { emu_hw_i80_set_read_callback (aCallback); }
void hw_i80_read (uint8_t cmd, uint8_t length) { emu_hw_i80_read (cmd, length); }
void hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *data) { emu_hw_i80_write (cmd, length, data); }
void hw_i80_reset (void) { emu_hw_i80_reset (); }

#endif /* MCODE_EMULATE_I80 */
