#ifndef MCODE_EMU_HW_UART_H
#define MCODE_EMU_HW_UART_H

/*typedef void (*hw_uart_char_event) (unsigned int aChar);*/
#include "hw-uart.h"

void emu_hw_uart_init (void);
void emu_hw_uart_deinit (void);

void emu_hw_uart_set_callback (hw_uart_char_event aCallback);

void emu_hw_uart_start_read (void);
void emu_hw_uart_write_string (const char *aString);

#endif /* MCODE_EMU_HW_UART_H */
