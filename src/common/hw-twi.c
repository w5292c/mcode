#include "cmd-engine.h"

#include "utils.h"
#include "hw-twi.h"
#include "hw-uart.h"
#include "mglobal.h"

static void i2c_callback(bool result);

#define BUFFER_LENGTH (20)
static uint8_t TheBuffer[BUFFER_LENGTH];

void cmd_engine_i2c_help(void)
{
  hw_uart_write_string_P(PSTR("> twi-rd <addr> <length> - Read <length> bytes from TWI device at <addr>\r\n"));
  hw_uart_write_string_P(PSTR("> twi-wr <addr> <hex-data> - Write <hex-data> to TWI device at <addr>\r\n"));
}

bool cmd_engine_i2c_read(const char *args)
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
  /* Parse the I2C address */
  int i2c_addr = 0;
  args = string_next_number(args, &i2c_addr);
  if (!args || !*args) {
    hw_uart_write_string_P(PSTR("Error: 2\r\n"));
    return false;
  }
  /* Parse the I2C read request length */
  int i2c_length = 0;
  args = string_skip_whitespace(args);
  args = string_next_number(args, &i2c_length);
  if (!args || *args || i2c_length <= 0 || i2c_length > 20) {
    hw_uart_write_string_P(PSTR("Error: wrong length: 0x"));
    hw_uart_write_uint(i2c_length);
    hw_uart_write_string_P(PSTR("\r\n"));
    return false;
  }

  hw_uart_write_string_P(PSTR("Arguments, I2C address: 0x"));
  hw_uart_write_uint(i2c_addr);
  hw_uart_write_string_P(PSTR(", length: 0x"));
  hw_uart_write_uint(i2c_length);
  hw_uart_write_string_P(PSTR("\"\r\n"));

  i2c_set_callback(i2c_callback);
  i2c_recv(i2c_addr, i2c_length);
  return true;
}

bool cmd_engine_i2c_write(const char *args)
{
  if (strncmp_P(args, PSTR("twi-wr "), 7)) {
    return false;
  }

  args += 7;
  /* Parse the I2C address */
  int i2c_addr = 0;
  args = string_next_number(args, &i2c_addr);
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
  hw_uart_write_uint(i2c_addr);
  hw_uart_write_string_P(PSTR(", write data:\r\n"));
  hw_uart_dump_buffer(bufferFilled, TheBuffer, true);

  i2c_set_callback(i2c_callback);
  i2c_send(i2c_addr, bufferFilled, TheBuffer);
  return true;
}

void i2c_callback(bool result)
{
  if (result) {
    hw_uart_write_string_P(PSTR("Success.\r\n"));
    hw_uart_dump_buffer(18, i2c_get_read_buffer(), true);
  } else {
    hw_uart_write_string_P(PSTR("Failed.\r\n"));
  }

  cmd_engine_start();
}
