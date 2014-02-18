#ifndef MCODE_HW_UART_H
#define MCODE_HW_UART_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*hw_uart_char_event) (unsigned int aChar);

void hw_uart_init (void);
void hw_uart_deinit (void);

void hw_uart_set_callback (hw_uart_char_event aCallback);

void hw_uart_start_read (void);

void hw_uart_write_uint (unsigned int value);
void hw_uart_write_string (const char *aString);
void hw_uart_write_string_P (const char *aString);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_HW_UART_H */
