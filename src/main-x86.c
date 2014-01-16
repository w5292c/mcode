#include "cmd-engine.h"

#include <stdio.h>
#include <signal.h>

static exit_request = 0;

static void main_at_exit (void);
static void main_sigint_handler (int signo);
static void main_line_callback (const char *aString);

int main (void)
{
  /* override the signal handler */
  if (SIG_ERR == signal(SIGINT, main_sigint_handler))
  {
    fprintf (stderr, "Error: cannot override signal handler\n");
    return -1;
  }
  atexit (main_at_exit);

  cmd_engine_init ();

  /* main dummy loop */
  while (!exit_request)
  {
    /* 0.1s = 100ms = 100000 us */
    usleep(100000);
  }

  cmd_engine_deinit ();
  pthread_exit (NULL);
  return 0;
}

static void main_at_exit (void)
{
  cmd_engine_deinit ();
}

void main_sigint_handler (int signo)
{
  if (SIGINT == signo)
  {
    printf ("MAIN: got exit signal\n");
    exit_request = 1;
  }
}

void main_request_exit (void)
{
  printf ("MAIN: exit request\n");
  exit_request = 1;
}
