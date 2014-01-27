#include "emu-hw-uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <avr/pgmspace.h>

static int running_request = 0;
static pthread_t TheKeyEventThread = 0;
static struct termios TheStoredTermIos;

/** The UART emulation thread */
static void *emu_hw_uart_thread (void *threadid);

void emu_hw_uart_init (void)
{
  if (!TheKeyEventThread)
  {
    /* get the original termios information */
    tcgetattr( STDIN_FILENO, &TheStoredTermIos);

    long t = 0;
    running_request = 1;
    const int failed = pthread_create(&TheKeyEventThread, NULL, emu_hw_uart_thread, (void *)t);
    if (failed)
    {
      running_request = 0;
      hw_uart_write_string_P (PSTR("Error: cannot create key-event thread, code: "));
      hw_uart_write_uint (failed);
      hw_uart_write_string_P (PSTR("\r\n"));
      exit (-1);
    }
  }
}

void emu_hw_uart_deinit (void)
{
  if (TheKeyEventThread)
  {
    int failed = pthread_cancel (TheKeyEventThread);
    if (failed)
    {
      hw_uart_write_string_P (PSTR("Error: cannot cancel thread, code: "));
      hw_uart_write_uint (failed);
      hw_uart_write_string_P (PSTR("r\n\"));
      exit (-1);
    }
    void *status;
    running_request = 0;
    failed = pthread_join(TheKeyEventThread, &status);
    if (failed)
    {
      hw_uart_write_string_P (PSTR("Error: cannot join key-event thread, code: "));
      hw_uart_write_uint (failed);
      hw_uart_write_string_P (PSTR("\r\n"));
      exit (-1);
    }

    /* restore the original termios attributes */
    tcsetattr( STDIN_FILENO, TCSANOW, &TheStoredTermIos);
    TheKeyEventThread = 0;
  }
}

void emu_hw_uart_start_read (void)
{
}

void emu_hw_uart_write_string_P (const char *aString)
{
  emu_hw_uart_write_string (aString);
}

void emu_hw_uart_write_string (const char *aString)
{
  puts (aString);
}

static hw_uart_char_event TheCallback = NULL;
void emu_hw_uart_set_callback (hw_uart_char_event aCallback)
{
  TheCallback = aCallback;
}

void *emu_hw_uart_thread (void *threadid)
{
  int escIndex = 0;
  uint escChar = 0;

  /* upate termios attributes, so, we receive getchar on each character entered, no echo */
  static struct termios newt;
  newt = TheStoredTermIos;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr( STDIN_FILENO, TCSANOW, &newt);

  while (running_request)
  {
    const unsigned int ch = getchar ();
    if (escIndex)
    {
      escChar = escChar | (ch << (8 * escIndex));
      ++escIndex;
    }
    if (27 == ch)
    {
      escChar = ch;
      escIndex = 1;
    }

    if (TheCallback && (escIndex == 3 || (!escIndex)))
    {
      if (escIndex)
      {
        (*TheCallback) (escChar);
        escIndex = 0;
      }
      else
      {
        (*TheCallback) (ch);
      }
    }
  }

  /* terminating thread */
  hw_uart_write_string_P (PSTR("emu_hw_uart_thread, finishing: "));
  hw_uart_write_uint (1234);
  hw_uart_write_string_P (PSTR("\r\n"));
  pthread_exit(NULL);
  return NULL;
}
