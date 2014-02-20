#include "scheduler.h"

#include "hw-uart.h"

#include <string.h>
#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include <gtk/gtk.h>
#include "emu-common.h"
#endif /* __AVR__ */

#define MCODE_TICKS_COUNT (8)

static int NoExitRequest;
static int ClientsNumber;
static mcode_cheduler_tick TheApplicationTicks[MCODE_TICKS_COUNT];

#ifdef __linux__
static gboolean mcode_scheduler_idle (gpointer user_data);
#endif /* __linux__ */

void mcode_scheduler_init (void)
{
  ClientsNumber = 0;
  NoExitRequest = 1;
  memset (TheApplicationTicks, 0, sizeof (TheApplicationTicks));

#ifdef __linux__
  g_idle_add ((GSourceFunc)mcode_scheduler_idle, NULL);
#endif /* __linux__ */
}

void mcode_scheduler_deinit (void)
{
  memset (TheApplicationTicks, 0, sizeof (TheApplicationTicks));
}

void mcode_scheduler_start (void)
{
#ifndef __linux__
  int i;
  while (NoExitRequest)
  {
    for (i = 0; i < ClientsNumber; i++)
    {
      mcode_cheduler_tick tick = TheApplicationTicks[i];
      if (tick)
      {
        (*tick) ();
      }
    }
  }
#else /* __linux__ */
  mcode_main_start ();
#endif /* __linux__ */
}

void mcode_scheduler_stop (void)
{
#ifndef __linux__
  NoExitRequest = 0;
#else /* __linux__ */
  mcode_main_quit ();
#endif /* __linux__ */
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

#ifdef __linux__
gboolean mcode_scheduler_idle (gpointer user_data)
{
  int i = 0;
  for (i = 0; i < ClientsNumber; i++)
  {
    mcode_cheduler_tick tick = TheApplicationTicks[i];
    if (tick)
    {
      (*tick) ();
    }
  }

  return TRUE;
}
#endif /* __linux__ */
