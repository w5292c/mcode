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

#include "cmd-engine.h"

#include "utils.h"
#include "hw-twi.h"
#include "hw-uart.h"
#include "mglobal.h"

static void twi_write_callback(bool result);
static void twi_read_callback(bool result, uint8_t length, const uint8_t *data);

#define BUFFER_LENGTH (20)
static uint8_t TheBuffer[BUFFER_LENGTH];

void cmd_engine_twi_help(void)
{
  hw_uart_write_string_P(PSTR("> twi-rd <addr> <length> - Read <length> bytes from TWI device at <addr>\r\n"));
  hw_uart_write_string_P(PSTR("> twi-wr <addr> <hex-data> - Write <hex-data> to TWI device at <addr>\r\n"));
}

bool cmd_engine_twi_read(const char *args)
{
  if (strncmp_P(args, PSTR("twi-rd "), 7)) {
    return false;
  }

  args += 7;

  /* move to the arguments start */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    hw_uart_write_string_P(PSTR("Error: 1\r\n"));
    return false;
  }
  /* Parse the TWI address */
  int twi_addr = 0;
  args = string_next_number(args, &twi_addr);
  if (!args || !*args) {
    hw_uart_write_string_P(PSTR("Error: 2\r\n"));
    return false;
  }
  /* Parse the TWI read request length */
  int twi_length = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &twi_length);
  if (!args || *args || twi_length <= 0 || twi_length > 20) {
    hw_uart_write_string_P(PSTR("Error: wrong length: 0x"));
    hw_uart_write_uint(twi_length);
    hw_uart_write_string_P(PSTR("\r\n"));
    return false;
  }

  hw_uart_write_string_P(PSTR("Arguments, TWI address: 0x"));
  hw_uart_write_uint(twi_addr);
  hw_uart_write_string_P(PSTR(", length: 0x"));
  hw_uart_write_uint(twi_length);
  hw_uart_write_string_P(PSTR("\"\r\n"));

  twi_set_read_callback(twi_read_callback);
  twi_recv(twi_addr, twi_length);
  return true;
}

bool cmd_engine_twi_write(const char *args)
{
  if (strncmp_P(args, PSTR("twi-wr "), 7)) {
    return false;
  }

  args += 7;
  /* Parse the TWI address */
  int twi_addr = 0;
  args = string_next_number(args, &twi_addr);
  if (!args || !*args) {
    return false;
  }

  uint8_t bufferFilled = 0;
  args = string_skip_whitespace(args);
  const char *end = string_to_buffer(args, BUFFER_LENGTH, TheBuffer, &bufferFilled);
  if (!end || *end || !bufferFilled) {
    hw_uart_write_string_P(PSTR("Wrong hex data: \""));
    hw_uart_write_string(args);
    hw_uart_write_string_P(PSTR("\"\r\n"));
    return false;
  }

  hw_uart_write_string_P(PSTR("TWI write, address: 0x"));
  hw_uart_write_uint(twi_addr);
  hw_uart_write_string_P(PSTR(", write data:\r\n"));
  hw_uart_dump_buffer(bufferFilled, TheBuffer, true);

  twi_set_write_callback(twi_write_callback);
  twi_send(twi_addr, bufferFilled, TheBuffer);
  return true;
}

void twi_read_callback(bool result, uint8_t length, const uint8_t *data)
{
  if (result) {
    hw_uart_write_string_P(PSTR("TWI read data:\r\n"));
    hw_uart_dump_buffer(length, data, true);
  } else {
    hw_uart_write_string_P(PSTR("TWI read failed.\r\n"));
  }

  cmd_engine_start();
}

void twi_write_callback(bool result)
{
  if (!result) {
    hw_uart_write_string_P(PSTR("TWI write failed.\r\n"));
  }

  cmd_engine_start();
}
