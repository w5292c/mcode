#include "hw-uart.h"

#include "mcode-config.h"

#include <stdlib.h>

#ifndef MCODE_EMULATE_UART
static hw_uart_char_event TheCallback = NULL;

void hw_uart_init (void)
{
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

void hw_uart_write_string (const char *aString)
{
}

#else /* MCODE_EMULATE_UART */
#include "emu-hw-uart.h"
/* for emulation, just forward the requests to the corresponding emulator implementation */
void hw_uart_init (void) { emu_hw_uart_init (); }
void hw_uart_deinit (void) { emu_hw_uart_deinit (); }
void hw_uart_set_callback (hw_uart_char_event aCallback) { emu_hw_uart_set_callback (aCallback); }
void hw_uart_start_read (void) { emu_hw_uart_start_read (); }
void hw_uart_write_string (const char *aString) { emu_hw_uart_write_string (aString); }
#endif /* MCODE_EMULATE_UART */
