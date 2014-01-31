#include "cmd-engine.h"

#include "main.h"
#include "hw-i80.h"
#include "hw-leds.h"
#include "hw-uart.h"
#include "line-editor-uart.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __AVR__
#include <avr/pgmspace.h>
#else /* __AVR__ */
#include "emu-common.h"
#endif /* __AVR__ */

static void cmd_engine_reset (void);
static uint8_t glob_is_hex_ch (unsigned char ch);
static void cmd_engine_read (const char *aCommand);
static void cmd_engine_write (const char *aCommand);
static void cmd_engine_set_led (const char *aCommand);
static unsigned char glob_ch_to_val (unsigned char ch);
static unsigned char glob_get_byte (const char *pData);
static void cmd_engine_on_cmd_ready (const char *aString);
static void cmd_engine_on_read_ready (int length, const unsigned char *pData);

void cmd_engine_init (void)
{
  hw_i80_init ();
  hw_i80_set_read_callback (cmd_engine_on_read_ready);
  line_editor_uart_init ();
  line_editor_uart_set_callback (cmd_engine_on_cmd_ready);
}

void cmd_engine_deinit (void)
{
  hw_i80_deinit ();
  line_editor_uart_deinit ();
}

void cmd_engine_start (void)
{
  line_editor_uart_start ();
}

void cmd_engine_on_cmd_ready (const char *aString)
{
  int start_uart_editor = 1;

  if (!strcmp_P (aString, PSTR("help")))
  {
    /* HELP command */
    hw_uart_write_string_P (PSTR("Supported cmds:\r\n"));
#if __linux__ == 1
    hw_uart_write_string_P (PSTR("> exit/quit - exit\r\n"));
#endif /* __linux__ == 1 */
    hw_uart_write_string_P (PSTR("> reset - Reset LCD module\r\n"));
    hw_uart_write_string_P (PSTR("> L <IND> <1/0> - Turn ON/OFF the LEDs\r\n"));
    hw_uart_write_string_P (PSTR("> W <CMD> <DAT> - write <CMD> with <DAT> to I80\r\n"));
    hw_uart_write_string_P (PSTR("> R <CMD> <LEN> - read <LEN> bytes with <CMD> in I80\r\n"));
  }
#ifdef __linux__
  else if (!strcmp_P (aString, PSTR("quit")) || !strcmp_P (aString, PSTR("exit")))
  {
    /* EXIT command */
    main_request_exit ();
    start_uart_editor = 0;
  }
#endif /* __X86__ */
  else if (!strncmp_P (aString, PSTR("W "), 2))
  {
    /* WRITE command */
    cmd_engine_write (&aString[2]);
    start_uart_editor = 0;
  }
  else if (!strncmp_P (aString, PSTR("R "), 2))
  {
    /* READ command */
    cmd_engine_read (&aString[2]);
    start_uart_editor = 0;
  }
  else if (!strncmp_P (aString, PSTR("L "), 2))
  {
    cmd_engine_set_led (&aString[2]);
  }
  else if (!strcmp_P (aString, PSTR("reset")))
  {
    cmd_engine_reset ();
  }
  else if (*aString)
  {
    /* got unrecognized non-empty command */
    hw_uart_write_string_P (PSTR("ENGINE: unrecognized command. Type 'help'.\r\n"));
  }

  if (start_uart_editor)
  {
    line_editor_uart_start ();
  }
}

void cmd_engine_read (const char *aCommand)
{
  int success = 1;
  unsigned char command = 0;
  unsigned char dataLength = 0;

  /* first, retrieve the command code */
  if (isxdigit (aCommand[0]) && isxdigit (aCommand[1]))
  {
    command = strtoul (aCommand, NULL, 16);
    aCommand += 2;
  }
  else
  {
    success = 0;
  }

  if (success)
  {
    /* skip whitespace */
    while (isspace(aCommand[0]))
    {
      ++aCommand;
    }

    const char ch0 = aCommand[0];
    if (ch0 && isxdigit (ch0))
    {
      char *end = NULL;
      dataLength = strtoul (aCommand, &end, 16);
      success = ((dataLength < 17) && end && !(*end));
      if (!success)
      {
        dataLength = 0;
      }
    }
    else if (ch0)
    {
      success = 0;
    }
  }

  if (success && dataLength)
  {
    /* execute the command on success and if we have non-zero data length */
    hw_i80_read (command, dataLength);
  }
  else
  {
    hw_uart_write_string_P (PSTR("Wrong args, format: R CC LL\r\n"));
    line_editor_uart_start ();
  }
}

/**
 * Set-LED command handler
 * @param[in] aCommand - The command parameters
 * @note The command parameters should go in the following format: "X Y";
 * X may be either 0 or 1 (LED index) and Y may be either 0 or 1 (OFF or ON).
 */
void cmd_engine_set_led (const char *aCommand)
{
  uint8_t success = 0;
  if (3 == strlen (aCommand))
  {
    const uint8_t ch0 = aCommand[0];
    const uint8_t ch2 = aCommand[2];

    if (glob_is_hex_ch (ch0) && glob_is_hex_ch (ch2) && ' ' == aCommand[1])
    {
      const uint8_t on = glob_ch_to_val (ch2);
      const uint8_t index = glob_ch_to_val (ch0);
      mcode_hw_leds_set (index, on);
      success = 1;
    }
  }

  if (!success)
  {
    hw_uart_write_string_P (PSTR("Wrong args, format: L I 0/1\r\n> I - the LED index [0..1]\r\n> 0/1 - turm OFF/ON\r\n"));
  }
}

void cmd_engine_reset (void)
{
  hw_i80_reset ();
}

void cmd_engine_on_read_ready (int length, const unsigned char *pData)
{
  hw_uart_write_string_P (PSTR("Got "));
  hw_uart_write_uint (length);
  hw_uart_write_string_P (PSTR(" bytes:\r\n"));

  int i;
  for (i = 0; i < length; ++i)
  {
    hw_uart_write_uint (pData[i]);
    if (i != length - 1)
    {
      hw_uart_write_string_P (PSTR(" "));
    }
  }
  if (length)
  {
    hw_uart_write_string_P (PSTR("\r\n"));
  }
  line_editor_uart_start ();
}

void cmd_engine_write (const char *aCommand)
{
  int success = 1;
  int dataLength = 0;
  unsigned char command;
#define MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH (32)
  unsigned char buffer[MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH];

  memset (buffer, 0, MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH);

  /* parse arguments */
  const unsigned char ch0 = aCommand[0];
  const unsigned char ch1 = aCommand[1];
  const unsigned char ch2 = aCommand[2];

  if (ch0 && ch1 && isxdigit (ch0) && isxdigit (ch1) && (' ' == ch2))
  {
    command = glob_get_byte (aCommand);
    aCommand += 3;

    while (*aCommand)
    {
      volatile uint8_t chH = aCommand[0];
      volatile uint8_t chL = aCommand[1];
      if (chH && chL && isxdigit (chH) && isxdigit (chL) && (dataLength < MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH - 1))
      {
        buffer[dataLength++] = (glob_ch_to_val (chH) << 4) | glob_ch_to_val (chL);
        aCommand += 2;
      }
      else
      {
        success = 0;
        break;
      }
    }
  }
  else
  {
    success = 0;
  }

  if (success)
  {
    /* pass the request to the I80 bus */
    hw_i80_write (command, dataLength, buffer);
  }
  else
  {
    hw_uart_write_string_P (PSTR("Wrong args, format: W CC X1[X2[X3..XA]]\r\n"));
  }
  /* restart the command line prompt */
  line_editor_uart_start ();
}

uint8_t glob_is_hex_ch (unsigned char ch)
{
  /* first, check if the character is a number */
  uint8_t res = (ch <= '9' && ch >= '0');
  if (!res)
  {
    /* convert to the upper case */
    ch &= ~0x20;
    /* and check if the character is [A-F] */
    res = (ch >= 'A' && ch <= 'F');
  }
  return res;
}

static unsigned char glob_ch_to_val (unsigned char ch)
{
  unsigned char value = ch;

  /* 'A' (10): 0x41, 0x61 */
  if (value > '9')
  {
    value = (value & (~0x20));
    value += 10 - 'A';
  }
  else
  {
    value -= '0';
  }

  return value;
}

unsigned char glob_get_byte (const char *pData)
{
  return (glob_ch_to_val (pData[0]) << 4) | glob_ch_to_val (pData[1]);
}
