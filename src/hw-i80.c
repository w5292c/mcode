#include "hw-i80.h"

#include "mcode-config.h"

#ifndef MCODE_EMULATE_I80
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/cpufunc.h>

/* HW configuration:
|---------------------------------------------------------------------------|
|  # | LCD Pin | UC Pin | Comment                                           |
|----|---------|--------|---------------------------------------------------|
|  1 |      D0 |     A0 |                                                   |
|  2 |      D1 |     A1 |                                                   |
|  3 |      D2 |     A2 |                                                   |
|  4 |      D3 |     A3 |                                                   |
|  5 |      D4 |     A4 |                                                   |
|  6 |      D5 |     A5 |                                                   |
|  7 |      D6 |     A6 |                                                   |
|  8 |      D7 |     A7 |                                                   |
|----|---------|--------|---------------------------------------------------|
|  9 |      CS |     B0 | TBC                                               |
| 10 |      WR |     B1 | TBC                                               |
| 11 |      RD |     B2 | TBC                                               |
| 12 |   A0/RS |     B3 | TBC                                               |
| 13 |   RESET |     B4 | TBC                                               |
|---------------------------------------------------------------------------|
*/
#define HW_I80_READ_BUFFER_LENGTH (16)
static unsigned char TheReadBuffer[HW_I80_READ_BUFFER_LENGTH];
static hw_i80_read_callback TheReadCallback = NULL;
static hw_i80_write_callback TheWriteCallback = NULL;

void hw_i80_init (void)
{
/*
#define PINA    _SFR_IO8(0x19)
#define DDRA    _SFR_IO8(0x1A)
#define PORTA   _SFR_IO8(0x1B)
*/
  /* make all the A-port pins as inputs */
  DDRA = 0x00U;
  /* disable pull-up resistors */
  PORTA = 0x00U;
  /* Configure B-port, outputs: CS, WR, RD, RS, RESET */
  DDRB = ((1U << DDB0)|(1U << DDB1)|(1U << DDB2)|(1U << DDB3)|(1U << DDB4));
  /* Reset all B-port outputs to inactive state (1) */
  PORTB = ((1U << PB0)|(1U << PB1)|(1U << PB2)|(1U << PB3)|(1U << PB4));
  _NOP ();
}

void hw_i80_deinit (void)
{
}

void hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
  TheReadCallback = aCallback;
}

void hw_i80_set_write_callback (hw_i80_write_callback aCallback)
{
  TheWriteCallback = aCallback;
}

void hw_i80_write (unsigned char cmd, int length, const unsigned char *data)
{
}

void hw_i80_read (unsigned char cmd, int length)
{
  if (length > HW_I80_READ_BUFFER_LENGTH)
  {
    /* we cannot read more than the length of the buffer we have */
    length = HW_I80_READ_BUFFER_LENGTH;
  }

  /* activate CS */
  PORTB &= ~(1U << PB0);

  /* activate D/CX */
  PORTB &= ~(1U << PB3);

  /* configure PORTA as outputs */
  DDRA = 0xFFU;
  /* send the command ID to the port A */
  PORTA = cmd;
  /* activate WR */
  PORTB &= (1U << PB1);
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
  /* deactivate WR */
  PORTB |= (1U << PB1);
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
  /* deactivate D/CX */
  PORTB |= (1U << PB3);
  /* confugure PORTA as input, do not enable pull-up resistors */
  DDRA = 0x00U;
  PORTA = 0x00U;
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
#if 1 /* incorrect read request, todo: check if this is required */
  /* activate WRX and RDX pins */
  PORTB &= ~((1U << PB1)|(1U << PB2));
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
  /* deactivate WRX and RDX */
  PORTB |= ((1U << PB1)|(1U << PB2));
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
#endif /* end: incorrect read request, todo: check if this is required */
  /* Read cycle */
  memset (TheReadBuffer, 0, HW_I80_READ_BUFFER_LENGTH);
  int i;
  for (i = 0; i < length; ++i)
  {
    /* correct reads, activate RDX */
    PORTB &= ~(1U << PB2);
    /* some delay, todo: check if this is required */
    _NOP (); _NOP (); _NOP (); _NOP ();
    /* Now, read the PORTA */
    unsigned char data = PINA;
    TheReadBuffer[i] = data;
    /* and deactivate RDX */
    PORTB |= (1U << PB2);
    /* some delay, todo: check if this is required */
    _NOP (); _NOP (); _NOP (); _NOP ();
  }

  /* now, move to the idle state */
  /* deactivate CS */
  PORTB |= (1U << PB0);

  /* and finally, notify the client */
  if (TheReadCallback)
  {
    (*TheReadCallback) (length, TheReadBuffer);
  }
}

#else /* MCODE_EMULATE_I80 */
void hw_i80_init (void) { emu_hw_i80_init (); }
void hw_i80_deinit (void) { emu_hw_i80_deinit (); }
void hw_i80_set_read_callback (hw_i80_read_callback aCallback) { emu_hw_i80_set_read_callback (aCallback); }
void hw_i80_set_write_callback (hw_i80_write_callback aCallback) { emu_hw_i80_set_write_callback (aCallback); }
void hw_i80_read (unsigned char cmd, int length) { emu_hw_i80_read (cmd, length); }
void hw_i80_write (unsigned char cmd, int length, const unsigned char *data) { emu_hw_i80_write (cmd, length, data); }

#endif /* MCODE_EMULATE_I80 */
