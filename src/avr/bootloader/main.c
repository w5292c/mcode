/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Alexander Chumakov
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

#include <avr/io.h>
#include <stdbool.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

static void bl_main_loop(void);
static uint8_t bl_uart_read_char(void);
static void bl_uart_write_char(uint8_t ch);

int main(void)
{
  /* Configure UART */
  /* Set baud rate: 115200 */
  UBRRH = (unsigned char)0;
  UBRRL = (unsigned char)3;
  UCSRA = 0;
  /* Enable receiver and transmitter */
  UCSRB = (1<<RXEN)|(1<<TXEN);
  /* Set frame format: 8data, 1stop bit */
  UCSRC = (1<<URSEL)|(3<<UCSZ0);

  bl_main_loop();
  return 0;
}

void bl_main_loop(void)
{
  /* For a 32K devices addresses are 16-bit.
     Should be updated for larger devices, if required. */
  uint16_t address = 0;
  uint16_t temp_int;
  uint8_t val;

  while (true) {
    const uint8_t ch = bl_uart_read_char();

    if (ch == 0) {
    }
    /* Request to set address */
    else if (ch == 'A') {
      address = (bl_uart_read_char()<<8) | bl_uart_read_char();
      bl_uart_write_char('\r'); // Send OK back.
    }
    /* Check for autoincrement */
    else if (ch == 'a') {
      /* Yes, we support autoincrement */
      bl_uart_write_char('Y');
    }
    /* Exit request, return to the application space */
    else if (ch == 'E') {
      void (*funcptr)(void) = 0x0000;

      boot_spm_busy_wait();
      boot_rww_enable();
      bl_uart_write_char('\r');
      funcptr();
    } else if (ch == 'e') {
      /* Chip erase */
      for (address = 0; address < 0x7000U; address += SPM_PAGESIZE) {
        eeprom_busy_wait();
        boot_spm_busy_wait();
        boot_page_erase(address);
      }
      bl_uart_write_char('\r');
    } else if(ch=='b') {
      bl_uart_write_char('Y');
      bl_uart_write_char((SPM_PAGESIZE>>8) & 0xFF);
      bl_uart_write_char(SPM_PAGESIZE&0xFF);
    } else if(ch=='B') {
      /* Start block load */
      temp_int = (bl_uart_read_char()<<8) | bl_uart_read_char();
      val = bl_uart_read_char();
      if (val == 'F') {
        const uint16_t baseAddr = (address<<1);
        uint16_t addr = baseAddr;

        do {
          const uint16_t data = bl_uart_read_char() | (bl_uart_read_char() << 8);
          boot_page_fill(addr, data);
          temp_int -= 2;
          addr += 2;
        } while (temp_int);
        boot_page_write(baseAddr);
        boot_spm_busy_wait();
        boot_rww_enable();
        address = addr >> 1;
        bl_uart_write_char('\r');
      } else if (val == 'E') {
      } else {
        bl_uart_write_char('?');
      }
/*      bl_uart_write_char(BlockLoad(temp_int, val, &address));*/
    } else if(ch=='g') {
      uint16_t i;
      /* Start block read */
      temp_int = (bl_uart_read_char()<<8) | bl_uart_read_char();
      val = bl_uart_read_char();
      if (val == 'F') {
        uint16_t addr = (address<<1);
        for (i=0; i < temp_int; ++i, ++addr) {
          bl_uart_write_char(pgm_read_byte_near(addr));
        }
      } else if (val == 'E') {
        for (i=0; i < temp_int; ++i) {
          bl_uart_write_char(eeprom_read_byte((void *)address++));
        }
      } else {
        bl_uart_write_char('?');
      }
    } else if (ch == 'S') {
      /* ISP-ID */
      bl_uart_write_char('A');
      bl_uart_write_char('V');
      bl_uart_write_char('R');
      bl_uart_write_char(' ');
      bl_uart_write_char('1');
      bl_uart_write_char('0');
      bl_uart_write_char('9');
    } else if (ch == 'V') {
      /* SW Version */
      bl_uart_write_char('1');
      bl_uart_write_char('0');
    } else if (ch == 'v') {
      /* HW Version */
      bl_uart_write_char('1');
      bl_uart_write_char('0');
    } else if (ch == 'p') {
      bl_uart_write_char('S');
    } else if (ch == 'a') {
      bl_uart_write_char('Y');
    } else if (ch == 't') {
      /* AVR910 Device Code: ATmega32 */
      bl_uart_write_char(0x72);
      /* Only 1 device is supported, end-of-list */
      bl_uart_write_char(0);
    } else if (ch == 'P' || ch == 'L') {
      /* Enter and Leave prog-mode, ignore */
      bl_uart_write_char('\r');
    } else if (ch == 'x' || ch == 'y' || ch == 'T') {
      /* Turn LED on/off, set device type: ignore requests for now */
      bl_uart_read_char();
      bl_uart_write_char('\r');
    } else if (ch == 's') {
      bl_uart_write_char(SIGNATURE_2);
      bl_uart_write_char(SIGNATURE_1);
      bl_uart_write_char(SIGNATURE_0);
    } else if (ch == 'r') {
      bl_uart_write_char(boot_lock_fuse_bits_get(GET_LOCK_BITS));
    } else if (ch == 'F') {
      bl_uart_write_char(boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS));
    } else if (ch == 'N') {
      bl_uart_write_char(boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS));
    } else {
      /* Unrecognized command */
      bl_uart_write_char('?');
    }
  }
}

uint8_t bl_uart_read_char(void)
{
  while (!(UCSRA & (1 << RXC)));
  return UDR;
}

void bl_uart_write_char(uint8_t ch)
{
  UDR = ch;
  while (!(UCSRA & (1 << TXC)));
  UCSRA |= (1 << TXC);
}
