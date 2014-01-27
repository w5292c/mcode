#include "hw-leds.h"
#include "hw-uart.h"
#include "scheduler.h"
#include "cmd-engine.h"
#include "line-editor-uart.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

int main (void)
{
  /* first, init the scheduler */
  mcode_scheduler_init ();
  /* now, UART can be initialized */
  hw_uart_init ();
  /* init LEDs */
  mcode_hw_leds_init ();
  /* init the line editor and the command engine */
  line_editor_uart_init ();
  cmd_engine_init ();

  /* now, enable the interrupts */
  sei ();

  /* Write some 'hello' text */
  hw_uart_write_string_P (PSTR("main: ready\r\nTest value: ["));
  hw_uart_write_uint (0x12afu);
  hw_uart_write_string_P (PSTR("]\r\n"));
  /* start the command engine, and switch to the scheduler, it never exits */
  cmd_engine_start ();
  mcode_scheduler_start ();
  return 0;
}
