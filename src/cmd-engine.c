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

static const char *TheHelpString =
"Supported cmds:\n"
#if __linux__ == 1
"> exit/quit - exit\n"
#endif /* __linux__ == 1 */
"> W <CMD> <DAT> - write <CMD> with <DAT> to I80\n"
"> R <CMD> <LEN> - read <LEN> bytes with <CMD> in I80\n";

static void cmd_engine_on_write_ready (int length);
static void cmd_engine_read (const char *aCommand);
static void cmd_engine_write (const char *aCommand);
static void cmd_engine_set_led (const char *aCommand);
static unsigned char glob_ch_to_val (unsigned char ch);
static void cmd_engine_on_cmd_ready (const char *aString);
static unsigned char glob_get_byte (const char *pData);
static void cmd_engine_on_read_ready (int length, const unsigned char *pData);

void cmd_engine_init (void)
{
  hw_i80_init ();
  hw_i80_set_read_callback (cmd_engine_on_read_ready);
  hw_i80_set_write_callback (cmd_engine_on_write_ready);
  line_editor_uart_init ();
  line_editor_uart_set_callback (cmd_engine_on_cmd_ready);
  line_editor_uart_start ();
}

void cmd_engine_deinit (void)
{
  hw_i80_deinit ();
  line_editor_uart_deinit ();
}

void cmd_engine_on_cmd_ready (const char *aString)
{
  int start_uart_editor = 1;

  if (!strcmp (aString, "help"))
  {
    /* HELP command */
    hw_uart_write_string (TheHelpString);
  }
#ifdef __linux__
  else if (!strcmp (aString, "quit") || !strcmp (aString, "exit"))
  {
    /* EXIT command */
    main_request_exit ();
    start_uart_editor = 0;
  }
#endif /* __X86__ */
  else if (!strncmp (aString, "W ", 2))
  {
    /* WRITE command */
    cmd_engine_write (&aString[2]);
    start_uart_editor = 0;
  }
  else if (!strncmp (aString, "R ", 2))
  {
    /* READ command */
    cmd_engine_read (&aString[2]);
    start_uart_editor = 0;
  }
  else if (!strncmp (aString, "L ", 2))
  {
    cmd_engine_set_led (&aString[2]);
  }
  else if (*aString)
  {
    /* got unrecognized non-empty command */
    hw_uart_write_string("ENGINE: unrecognized command. Type 'help'.\n");
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
      success = ((dataLength < 11) && end && !(*end));
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
    hw_uart_write_string ("Wrong args, format: R CC LL\n");
    line_editor_uart_start ();
  }
}

void cmd_engine_set_led (const char *aCommand)
{
  int on = 0;
  int index = 0;
  const int n = 2;
//sscanf (aCommand, "%d %d", &index, &on);

  if (2 == n)
  {
#ifdef __AVR_MEGA__
    mcode_hw_leds_set (index, on);
#else /* __AVR_MEGA__ */
    char buffer[100];
    snprintf (buffer, sizeof (buffer), "Setting LED#%d: %s\n", index, on ? "ON" : "OFF");
    hw_uart_write_string (buffer);
#endif /* __AVR_MEGA__ */
  }
  else
  {
    hw_uart_write_string ("Wrong args, format: L IND ON\n");
  }
}

void cmd_engine_on_read_ready (int length, const unsigned char *pData)
{
  int i;
  char buffer[16];

  snprintf (buffer, 16, "Got %d bytes:", length);
  hw_uart_write_string (buffer);
  hw_uart_write_string ("\n>> ");
  for (i = 0; i < length; ++i)
  {
    snprintf (buffer, 16, "%2.2X%s", pData[i], (i == length - 1) ? "" : " ");
    hw_uart_write_string (buffer);
  }
  if (length)
  {
    hw_uart_write_string ("\n");
  }
  line_editor_uart_start ();
}

void cmd_engine_write (const char *aCommand)
{
  /*int index = 0;*/
  int success = 1;
  int dataLength = 0;
  unsigned char command;
  unsigned char buffer[16];

  memset (buffer, 0, 16);

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
      unsigned char chH = aCommand[0];
      unsigned char chL = aCommand[1];
      if (chH && chL && isxdigit (chH) && isxdigit (chL))
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
    hw_uart_write_string ("Wrong args, format: W CC X1[X2[X3..XA]]\n");
    line_editor_uart_start ();
  }
}

void cmd_engine_on_write_ready (int length)
{
  line_editor_uart_start ();
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
