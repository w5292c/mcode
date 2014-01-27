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
static hw_i80_write_callback TheWriteCallback = NULL;

void hw_i80_init (void)
{
  /* make all the A-port pins as inputs */
  DDRA = 0x00U;
  /* disable pull-up resistors */
  PORTA = 0x00U;
  /* Configure C and D-ports, outputs: CS, WR, RD, RS, RESET */
  DDRD = (1U << DDD7);
  DDRC = ((1U << DDC0)|(1U << DDC1)|(1U << DDC6)|(1U << DDC7));
  /* Reset all C and D-port outputs to inactive state (1) */
  PORTD = (1U << PD7);
  PORTC = ((1U << PC0)|(1U << PC1)|(1U << PC6)|(1U << PC7));
  _NOP ();
}

void hw_i80_deinit (void)
{
  /* make all the A-port pins as inputs */
  DDRA = 0x00U;
  /* disable pull-up resistors */
  PORTA = 0x00U;
  /* Configure C and D-port, outputs: CS, WR, RD, RS, RESET */
  /**@todo check if we need to store configuration for other pins */
  DDRD = (1U << DDD7);
  DDRC = ((1U << DDC0)|(1U << DDC1)|(1U << DDC6)|(1U << DDC7));
  /* Reset all C and D-port outputs to inactive state (1) */
  PORTD = (1U << PD7);
  PORTC = ((1U << PC0)|(1U << PC1)|(1U << PC6)|(1U << PC7));
  _NOP ();
}

void hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
  TheReadCallback = aCallback;
}

void hw_i80_set_write_callback (hw_i80_write_callback aCallback)
{
  TheWriteCallback = aCallback;
}

void hw_i80_write (unsigned char cmd, int length, const unsigned char *pData)
{
  /* activate CS */
  PORTC &= ~(1U << PC0);

  /* activate D/CX */
  PORTC &= ~(1U << PC7);
  /* configure PORTA as outputs */
  DDRA = 0xFFU;
  /* send the command ID to the port A */
  PORTA = cmd;
  /* activate WR */
  PORTC &= ~(1U << PC1);
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
  /* deactivate WR */
  PORTC |= (1U << PC1);
  /* deactivate D/CX */
  PORTC |= (1U << PC7);
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();

  /* write cycle */
  int i;
  for (i = 0; i < length; ++i)
  {
    const unsigned char data = pData[i];
    PORTA = data;
    PORTC &= ~(1U << PC1);
    /* some delay, todo: check if this is required */
    _NOP (); _NOP (); _NOP (); _NOP ();
    PORTC |= (1U << PC1);
    /* some delay, todo: check if this is required */
    _NOP (); _NOP (); _NOP (); _NOP ();
  }

  /* clean-up */
  /* configure PORTA as inputs */
  DDRA = 0x00U;
  PORTA = 0x00U;
  /* deactivate CS */
  PORTC |= (1U << PC0);
}

void hw_i80_read (unsigned char cmd, int length)
{
  if (length > HW_I80_READ_BUFFER_LENGTH)
  {
    /* we cannot read more than the length of the buffer we have */
    length = HW_I80_READ_BUFFER_LENGTH;
  }

  /* activate CS */
  PORTC &= ~(1U << PC0);

  /* activate D/CX */
  PORTC &= ~(1U << PC7);

  /* configure PORTA as outputs */
  DDRA = 0xFFU;
  /* send the command ID to the port A */
  PORTA = cmd;
  /* activate WR */
  PORTC &= ~(1U << PC1);
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
  /* deactivate WR */
  PORTC |= (1U << PC1);
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
  /* deactivate D/CX */
  PORTC |= (1U << PC7);
  /* confugure PORTA as input, do not enable pull-up resistors */
  DDRA = 0x00U;
  PORTA = 0x00U;
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
#if 0 /* incorrect read request, todo: check if this is required */
  /* activate WRX and RDX pins */
  PORTC &= ~((1U << PC1)|(1U << PC6));
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
  /* deactivate WRX and RDX */
  PORTC |= ((1U << PC1)|(1U << PC6));
  /* some delay, todo: check if this is required */
  _NOP (); _NOP (); _NOP (); _NOP ();
#endif /* end: incorrect read request, todo: check if this is required */
  /* Read cycle */
  memset (TheReadBuffer, 0, HW_I80_READ_BUFFER_LENGTH);
  int i;
  for (i = 0; i < length; ++i)
  {
    /* correct reads, activate RDX */
    PORTC &= ~(1U << PC6);
    /* some delay, todo: check if this is required */
    _NOP (); _NOP (); _NOP (); _NOP ();
    /* Now, read the PORTA */
    unsigned char data = PINA;
    TheReadBuffer[i] = data;
    /* and deactivate RDX */
    PORTC |= (1U << PC6);
    /* some delay, todo: check if this is required */
    _NOP (); _NOP (); _NOP (); _NOP ();
  }

  /* now, move to the idle state */
  /* deactivate CS */
  PORTC |= (1U << PC0);

  /* and finally, notify the client */
  if (TheReadCallback)
  {
    (*TheReadCallback) (length, TheReadBuffer);
  }
}

#else /* MCODE_EMULATE_I80 */
#include "emu-hw-i80.h"

void hw_i80_init (void) { emu_hw_i80_init (); }
void hw_i80_deinit (void) { emu_hw_i80_deinit (); }
void hw_i80_set_read_callback (hw_i80_read_callback aCallback) { emu_hw_i80_set_read_callback (aCallback); }
void hw_i80_set_write_callback (hw_i80_write_callback aCallback) { emu_hw_i80_set_write_callback (aCallback); }
void hw_i80_read (unsigned char cmd, int length) { emu_hw_i80_read (cmd, length); }
void hw_i80_write (unsigned char cmd, int length, const unsigned char *data) { emu_hw_i80_write (cmd, length, data); }

#endif /* MCODE_EMULATE_I80 */
