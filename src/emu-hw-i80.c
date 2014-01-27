#include "emu-hw-i80.h"

#include "hw-uart.h"
#include "mcode-config.h"

#ifdef MCODE_EMULATE_I80_LCD1
#include "emu-hw-i80-lcd1.h"
#endif

#ifndef MCODE_EMULATE_I80
#error "The file 'emu-hw-i80.c' can only be included in MCODE_EMULATE_I80 mode"
#endif /* MCODE_EMULATE_I80 */

void emu_hw_i80_init (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_init ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_init: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_deinit (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_deinit ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_deinit: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_set_read_callback (aCallback);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_set_read_callback ("));
  hw_uart_write_uint (aCallback);
  hw_uart_write_string_P (PSTR("): no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_set_write_callback (hw_i80_write_callback aCallback)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_set_write_callback (aCallback);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_set_write_callback ("));
  hw_uart_write_uint (aCallback);
  hw_uart_write_string_P (PSTR("): no device connected\r\n");
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write (unsigned char cmd, int length, const unsigned char *data)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_write (cmd, length, data);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_read (unsigned char cmd, int length)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_read (cmd, length);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_read: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}
