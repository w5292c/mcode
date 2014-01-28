#include "scheduler.h"

#include "hw-uart.h"

#include <string.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

#define MCODE_TICKS_COUNT (8)

static int NoExitRequest;
static int ClientsNumber;
static mcode_cheduler_tick TheApplicationTicks[MCODE_TICKS_COUNT];

void mcode_scheduler_init (void)
{
  ClientsNumber = 0;
  NoExitRequest = 1;
  memset (TheApplicationTicks, 0, sizeof (TheApplicationTicks));
}

void mcode_scheduler_deinit (void)
{
  memset (TheApplicationTicks, 0, sizeof (TheApplicationTicks));
}

void mcode_scheduler_start (void)
{
  int i;
  while (NoExitRequest)
  {
    for (i = 0; i < ClientsNumber; i++)
    {
      mcode_cheduler_tick tick = TheApplicationTicks[i];
      if (tick)
      {
        (*tick) ();
#ifdef __X86__
        usleep(1000);
#endif /* __X86__ */
      }
    }
  }
}

void mcode_scheduler_stop (void)
{
  NoExitRequest = 0;
}

void mcode_scheduler_add (mcode_cheduler_tick tick)
{
  if (ClientsNumber < MCODE_TICKS_COUNT)
  {
    TheApplicationTicks[ClientsNumber++] = tick;
  }
  else
  {
    hw_uart_write_string_P(PSTR("ERROR: scheduler: no space\r\n"));
  }
}
