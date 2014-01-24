#include "hw-uart.h"

#include "scheduler.h"
#include "mcode-config.h"
#include "line-editor-uart.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef MCODE_EMULATE_UART
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define HW_UART_READ_BUFFER_LENGTH (64)
#define HW_UART_WRITE_BUFFER_LENGTH (128)

static hw_uart_char_event TheCallback = NULL;

static uint8_t TheCurrentBuffer = 0;
static unsigned char TheReadBuffer1[HW_UART_READ_BUFFER_LENGTH];
static unsigned char TheReadBuffer2[HW_UART_READ_BUFFER_LENGTH];

static unsigned int TheWriteBufferEnd = 0;
static unsigned int TheWriteBufferStart = 0;
static unsigned char TheWriteBuffer[HW_UART_WRITE_BUFFER_LENGTH];

static void hw_uart_tick (void);

void hw_uart_init (void)
{
  TheWriteBufferEnd = 0;
  TheWriteBufferStart = 0;
  TheCurrentBuffer = 0;
  memset (TheReadBuffer1, 0, HW_UART_READ_BUFFER_LENGTH);
  memset (TheReadBuffer2, 0, HW_UART_READ_BUFFER_LENGTH);
  memset (TheWriteBuffer, 0, HW_UART_WRITE_BUFFER_LENGTH);

#ifdef __AVR_MEGA__
  /* Set baud rate: 115200 */
  UBRRH = (unsigned char)0;
  UBRRL = (unsigned char)3;
  /* Enable receiver and transmitter */
  UCSRB = (1<<RXEN)|(1<<TXEN);
  /* Set frame format: 8data, 2stop bit */
  UCSRC = (1<<URSEL)|(3<<UCSZ0);
#endif /* __AVR_MEGA__ */
  mcode_scheduler_add (hw_uart_tick);
}

void hw_uart_deinit (void)
{
}

void hw_uart_set_callback (hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void hw_uart_start_read (void)
{
}

static int8_t vtoch (uint8_t value)
{
  value = value & 0x0FU;
  if (value < 10 && value >= 0)
  {
    return '0' + value;
  }
  else if (value >= 10 && value < 16)
  {
    return 'A' + value - 10;
  }
  else
  {
    return '@';
  }
}

void hw_uart_write_uint (unsigned int value)
{
  value = value & 0xFFFFU;
  char buffer[6];
  buffer[0] = '#';
  buffer[1] = vtoch (0x0FU & (value >> 12));
  buffer[2] = vtoch (0x0FU & (value >>  8));
  buffer[3] = vtoch (0x0FU & (value >>  4));
  buffer[4] = vtoch (0x0FU & value);
  buffer[5] = 0;
  hw_uart_write_string (buffer);
}

void hw_uart_write_string (const char *aString)
{
/*  if (TheWriteBufferStart)
  {
    memmove (&TheWriteBuffer[0], &TheWriteBuffer[TheWriteBufferStart], TheWriteBufferStart);

    TheWriteBufferEnd -= TheWriteBufferStart;
    TheWriteBufferStart = 0;
  }*/

  uint8_t ch;
  while (0 != (ch = *aString))
  {
    while ( !( UCSRA & (1<<UDRE)) ) ;
    UDR = ch;

    ++aString;
  }

#if 0
  uint8_t i;
  const uint8_t length = strlen (aString);
  for (i = 0; i < length; ++i)
  {
    /* Wait for empty transmit buffer */
    while ( !( UCSRA & (1<<UDRE)) ) ;
    /* Put data into buffer, sends the data */
    UDR = aString[i];
  }
#endif

/*  if (length)
  {
    const int freeBufferLength = HW_UART_WRITE_BUFFER_LENGTH - (TheWriteBufferEnd - TheWriteBufferStart);
    if (length > freeBufferLength)
    {
      length = freeBufferLength;
    }

    memcpy (&TheWriteBuffer[TheWriteBufferEnd], aString, length);
    TheWriteBufferEnd += length;
  }*/
}

void hw_uart_write_string_P (const char *aString)
{
  uint8_t ch;
  while (0 != (ch = pgm_read_byte (aString)))
  {
    while ( !( UCSRA & (1<<UDRE)) );
    UDR = ch;

    ++aString;
  }
/* pgm_read_byte */
}

static void hw_uart_tick (void)
{
/*  const int bufferBytes = (TheWriteBufferEnd - TheWriteBufferStart);
  if (bufferBytes)
  {
    fprintf (stdout, "%c", (char)TheWriteBuffer[TheWriteBufferStart++]);
    fflush (stdout);
  }*/

#if 0 /* test code */
  static int n = 0;
  if (++n == 20)
  {
    hw_uart_write_string ("2 secs passed.\n");
    line_editor_uart_start ();
    n = 0;
  }
#endif /* test code */
}

#ifdef __AVR_ATmega32__
/* USART, Rx Complete */
ISR(USART_RXC_vect)
{
}

/* USART Data Register Empty */
ISR(USART_UDRE_vect)
{
}

/* USART, Tx Complete */
ISR(USART_TXC_vect)
{
}
#endif /* __AVR_ATmega32__ */

#else /* MCODE_EMULATE_UART */
#include "emu-hw-uart.h"
/* for emulation, just forward the requests to the corresponding emulator implementation */
void hw_uart_init (void) { emu_hw_uart_init (); }
void hw_uart_deinit (void) { emu_hw_uart_deinit (); }
void hw_uart_set_callback (hw_uart_char_event aCallback) { emu_hw_uart_set_callback (aCallback); }
void hw_uart_start_read (void) { emu_hw_uart_start_read (); }
void hw_uart_write_string (const char *aString) { emu_hw_uart_write_string (aString); }
#endif /* MCODE_EMULATE_UART */
