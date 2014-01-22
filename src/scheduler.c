#include "scheduler.h"

#include <string.h>

#define MCODE_TICKS_COUNT (8)

static int NoExitRequest;
static mcode_cheduler_tick TheApplicationTicks[MCODE_TICKS_COUNT];

void mcode_scheduler_init (void)
{
  NoExitRequest = 1;
  memset (TheApplicationTicks, 0, sizeof (TheApplicationTicks));
}

void mcode_scheduler_deinit (void)
{
  memset (TheApplicationTicks, 0, sizeof (TheApplicationTicks));
}

void mcode_scheduler_start (void)
{
  while (NoExitRequest)
  {
    int i;
    for (i = 0; i < MCODE_TICKS_COUNT; ++i)
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
  int i;
  for (i = 0; i < MCODE_TICKS_COUNT; ++i)
  {
    if (!TheApplicationTicks[i])
    {
      TheApplicationTicks[i] = tick;
      break;
    }
  }
}
