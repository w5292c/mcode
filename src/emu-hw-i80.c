#include "emu-hw-i80.h"

#include "hw-uart.h"
#include "mcode-config.h"

#ifdef MCODE_EMULATE_I80_LCD1
#include "emu-hw-i80-lcd1.h"
#elif defined (MCODE_EMULATE_LCD_S95513)
#include "emu-hw-lcd-s95513.h"
#endif

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

#ifndef MCODE_EMULATE_I80
#error "The file 'emu-hw-i80.c' can only be included in MCODE_EMULATE_I80 mode"
#endif /* MCODE_EMULATE_I80 */

void emu_hw_i80_init (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_init ();
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_init ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_init: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_deinit (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_deinit ();
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_deinit ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_deinit: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_set_read_callback (hw_i80_read_callback aCallback)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_set_read_callback (aCallback);
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_set_read_callback (aCallback);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_set_read_callback ("));
  hw_uart_write_uint (((uint64_t)(aCallback)));
  hw_uart_write_string_P (PSTR("): no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_write (uint8_t cmd, uint8_t length, const uint8_t *data)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_write (cmd, length, data);
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_write (cmd, length, data);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_write: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_read (uint8_t cmd, uint8_t length)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_read (cmd, length);
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_read (cmd, length);
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_read: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}

void emu_hw_i80_reset (void)
{
#ifdef MCODE_EMULATE_I80_LCD1
  emu_lcd1_hw_i80_reset ();
#elif defined (MCODE_EMULATE_LCD_S95513)
  emu_hw_lcd_s95513_reset ();
#else /* MCODE_EMULATE_I80_LCD1 */
  hw_uart_write_string_P (PSTR("Warning: emu_hw_i80_reset: no device connected\r\n"));
#endif /* MCODE_EMULATE_I80_LCD1 */
}
