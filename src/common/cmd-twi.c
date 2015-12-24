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
#include "strings.h"

static void twi_write_callback(bool result);
static bool cmd_engine_twi_read(const char *args, bool *startCmd);
static bool cmd_engine_twi_write(const char *args, bool *startCmd);
static void twi_read_callback(bool result, uint8_t length, const uint8_t *data);

#define BUFFER_LENGTH (20)
static uint8_t TheBuffer[BUFFER_LENGTH];

void cmd_engine_twi_help(void)
{
  hw_uart_write_string_P(PSTR("> twi-rd <addr> <length> - Read <length> bytes from TWI device at <addr>\r\n"));
  hw_uart_write_string_P(PSTR("> twi-wr <addr> <hex-data> - Write <hex-data> to TWI device at <addr>\r\n"));
}

bool cmd_engine_twi_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("twi-rd "), 7)) {
    return cmd_engine_twi_read(command + 7, startCmd);
  } else if (!strncmp_P(command, PSTR("twi-wr "), 7)) {
    return cmd_engine_twi_write(command + 7, startCmd);
  }

  return false;
}

bool cmd_engine_twi_read(const char *args, bool *startCmd)
{
  /* move to the arguments start */
  args = string_skip_whitespace(args);
  if (!args || !*args) {
    hw_uart_write_string_P(PSTR("Error: 1\r\n"));
    return true;
  }
  /* Parse the TWI address */
  uint16_t twi_addr = 0;
  args = string_next_number(args, &twi_addr);
  if (!args || !twi_addr) {
    merror(MStringWrongArgument);
    return true;
  }
  /* Parse the TWI read request length */
  uint16_t twi_length = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &twi_length);
  if (args || !twi_length || twi_length > 20) {
    merror(MStringWrongArgument);
    return true;
  }

  hw_uart_write_string_P(PSTR("Arguments, TWI address: 0x"));
  hw_uart_write_uint(twi_addr);
  hw_uart_write_string_P(PSTR(", length: 0x"));
  hw_uart_write_uint(twi_length);
  mprint(MStringNewLine);

  *startCmd = false;
  twi_set_read_callback(twi_read_callback);
  twi_recv(twi_addr, twi_length);
  return true;
}

bool cmd_engine_twi_write(const char *args, bool *startCmd)
{
  /* Parse the TWI address */
  uint16_t twi_addr = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &twi_addr);
  if (!args || !twi_addr) {
    merror(MStringWrongArgument);
    return true;
  }

  uint8_t bufferFilled = 0;
  args = string_skip_whitespace(args);
  args = string_to_buffer(args, BUFFER_LENGTH, TheBuffer, &bufferFilled);
  if (args || !bufferFilled) {
    merror(MStringWrongArgument);
    return true;
  }

  hw_uart_write_string_P(PSTR("TWI write, address: 0x"));
  hw_uart_write_uint(twi_addr);
  hw_uart_write_string_P(PSTR(", write data:\r\n"));
  hw_uart_dump_buffer(bufferFilled, TheBuffer, true);

  *startCmd = false;
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
