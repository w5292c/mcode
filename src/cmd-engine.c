#include "cmd-engine.h"

#include "main.h"
#include "line-editor-uart.h"

#include <stdlib.h>

static const char *TheHelpString =
"Supported cmds:\n"
"> exit/quit - exit\n"
"> W <CMD> <DAT> - write <CMD> with <DAT> to I80\n"
"> R <CMD> <LEN> - read <LEN> bytes with <CMD> in I80\n";

static void cmd_engine_read (const char *aCommand);
static void cmd_engine_write (const char *aCommand);
static void cmd_engine_on_cmd_ready (const char *aString);

void cmd_engine_init (void)
{
  line_editor_uart_init ();
  line_editor_uart_set_callback (cmd_engine_on_cmd_ready);
  line_editor_uart_start ();
}

void cmd_engine_deinit (void)
{
  line_editor_uart_deinit ();
}

void cmd_engine_on_cmd_ready (const char *aString)
{
  int start_uart_editor = 1;

  if (!strcmp (aString, "quit") || !strcmp (aString, "exit"))
  {
    /* EXIT command */
    main_request_exit ();
    start_uart_editor = 0;
  }
  else if (!strcmp (aString, "help"))
  {
    /* HELP command */
    hw_uart_write_string (TheHelpString);
  }
  else if (!strncmp (aString, "W ", 2))
  {
    /* WRITE command */
    cmd_engine_write (&aString[2]);
    start_uart_editor = 1;
  }
  else if (!strncmp (aString, "R ", 2))
  {
    /* READ command */
    cmd_engine_read (&aString[2]);
    start_uart_editor = 1;
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

  if (success)
  {
    /**@todo Implement reading. */
    /*printf ("Cmd: [0x%2.2X], length: [%d]\n", command, dataLength);*/
  }
  else
  {
    hw_uart_write_string ("Wrong args, format: R X1 X2: Read X2 (<= 10) bytes, X1 cmd\n");
  }
}

void cmd_engine_write (const char *aCommand)
{
  hw_uart_write_string ("Writing, arguments: [");
  hw_uart_write_string (aCommand);
  hw_uart_write_string ("]...\nDone.\n");
}
