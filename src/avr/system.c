/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Alexander Chumakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "system.h"

#include "mtick.h"
#include "hw-uart.h"
#include "strings.h"

#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

void reboot(void)
{
  mprintstrln(PSTR("System reboot has been requested."));
  /* Disable all interrupts */
  cli();
  /* Enable the Watchdog timer with the smallest timeout */
  wdt_enable(WDTO_15MS);
  /* Peacefully wait for the system reset */
  for(;;);
}

void bootloader(void)
{
  mprintstrln(PSTR("Bootloader requested."));
  typedef void (*boot)(void);
  const uint8_t hfuse = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
  const boot vector = (boot)((hfuse & ~FUSE_BOOTSZ1) ?
    ((hfuse & ~FUSE_BOOTSZ0) ? 0x7e00u : 0x7c00u) :
    ((hfuse & ~FUSE_BOOTSZ0) ? 0x7800u : 0x7000u));
  mprintstr(PSTR("Bootloader vector: 0x"));
  hw_uart_write_uint32((uint32_t)(uint16_t)vector, false);
  mprintln(MStringNull);
  vector();
}
