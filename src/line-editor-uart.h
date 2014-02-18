#ifndef MCODE_LINE_EDITOR_UART_H
#define MCODE_LINE_EDITOR_UART_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*line_editor_uart_ready) (const char *aString);

void line_editor_uart_init (void);
void line_editor_uart_deinit (void);

void line_editor_uart_set_callback (line_editor_uart_ready aCallback);


void line_editor_uart_start (void);
#if 0
void line_editor_uart_stop (void);
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_LINE_EDITOR_UART_H */
