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
#include "hw-i80.h"
#include "hw-uart.h"
#include "mglobal.h"

#include <string.h>

#ifdef MCODE_HW_I80_ENABLED
static void cmd_engine_read(const char *args, bool *startCmd);
static void cmd_engine_write(const char *args, bool *startCmd);

void cmd_engine_i80_help(void)
{
  hw_uart_write_string_P(PSTR("> i80-w <CMD> <DAT> - Write <CMD> with <DAT> to I80\r\n"));
  hw_uart_write_string_P(PSTR("> i80-r <CMD> <LEN> - Read <LEN> bytes with <CMD> in I80\r\n"));
}

bool cmd_engine_i80_command(const char *command, bool *startCmd)
{
  if (!strncmp_P(command, PSTR("i80-w "), 6)) {
    cmd_engine_write(&command[6], startCmd);
    return true;
  } else if (!strncmp_P(command, PSTR("i80-r "), 6)) {
    cmd_engine_read(&command[6], startCmd);
    return true;
  }

  return false;
}

void cmd_engine_read(const char *args, bool *startCmd)
{
  uint8_t buffer[16];
  uint8_t command = 0;
  uint8_t dataLength = 0;

  memset(buffer, 0, 16);
  /* first, retrieve the command code */
  int value = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &value);
  command = (uint8_t)value;
  args = string_skip_whitespace(args);

  if (args) {
    value = 0;
    string_next_number(args, &value);
    dataLength = (uint8_t)value;
  }

  /* Our read buffer is 16 bytes long */
  if (dataLength > 16) {
    dataLength = 16;
  }

  if (dataLength) {
    hw_i80_read(command, dataLength, buffer);

    /* Display the result */
    hw_uart_write_string_P(PSTR("Got "));
    hw_uart_write_uint(dataLength);
    hw_uart_write_string_P(PSTR(" bytes:\r\n"));
    hw_uart_dump_buffer(dataLength, buffer, true);
  }
}

void cmd_engine_write(const char *args, bool *startCmd)
{
  int dataLength = 0;
#define MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH (32)
  unsigned char buffer[MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH];
  memset(buffer, 0, MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH);

  int command = -1;
  /* Get the <command> first */
  args = string_skip_whitespace(args);
  args = string_next_number(args, &command);
  if (!args) {
    hw_uart_write_string_P(PSTR("Wrong args\r\n"));
    return;
  }
  /* Now, get the <command-data> */
  uint8_t bufferFilled;
  args = string_to_buffer(args, MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH, buffer, &bufferFilled);
  if (args || !bufferFilled) {
    hw_uart_write_string_P(PSTR("Wrong args\r\n"));
    return;
  }

  /* pass the request to the I80 bus */
  hw_i80_write(command, dataLength, buffer);
  hw_uart_write_string_P(PSTR("Done.\r\n"));
}
#endif /* MCODE_HW_I80_ENABLED */
