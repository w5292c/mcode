#include "emu-hw-uart.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>

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
      fprintf (stderr, "Error: cannot create key-event thread, code: %d\n", failed);
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
      fprintf (stderr, "Error: cannot cancel thread, code: %d\n", failed);
      exit (-1);
    }
    void *status;
    running_request = 0;
    failed = pthread_join(TheKeyEventThread, &status);
    if (failed)
    {
      fprintf (stderr, "Error: cannot join key-event thread, code: %d\n", failed);
      exit (-1);
    }

    /* restore the original termios attributes */
    tcsetattr( STDIN_FILENO, TCSANOW, &TheStoredTermIos);
    TheKeyEventThread = 0;
  }
}

void emu_hw_uart_start_read (void)
{
  printf ("emu_hw_uart_start_read\n");
}

void emu_hw_uart_write_string (const char *aString)
{
  printf ("%s", aString);
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

  /*printf ("emu_hw_uart_thread, starting: %d, %d\n", 1234, running_request);*/

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
  printf ("emu_hw_uart_thread, finishing: %d\n", 1234);
  pthread_exit(NULL);
  return NULL;
}
