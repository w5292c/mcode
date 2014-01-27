#include "hw-leds.h"
#include "hw-uart.h"
#include "scheduler.h"
#include "cmd-engine.h"
#include "line-editor-uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

static void main_tick (void);

int main (void)
{
  mcode_scheduler_init ();
  hw_uart_init ();

  /* init LEDs */
  mcode_hw_leds_init ();
  mcode_hw_leds_set (0, 0);
  mcode_hw_leds_set (1, 1);

  hw_uart_write_string ("main: ready\r\nvalue: ");
  hw_uart_write_uint (0x12afu);
  hw_uart_write_string ("\r\n");

  mcode_scheduler_add (main_tick);
  line_editor_uart_init ();
  cmd_engine_init ();

  /* now, enable the interrupts */
  sei ();
  /* start never exits */
  mcode_scheduler_start ();
  return 0;
}

void main_tick (void)
{
  static int n = 0;

  if (n++ == 0x1FFF)
  {
    mcode_hw_leds_set (0, !mcode_hw_leds_get (0));
    mcode_hw_leds_set (1, !mcode_hw_leds_get (1));
    n = 0;
  }
}
