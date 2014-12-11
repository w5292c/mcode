/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Alexander Chumakov
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

#include "main.h"
#include "hw-i80.h"
#include "console.h"
#include "hw-leds.h"
#include "hw-uart.h"
#include "hw-lcd-s95513.h"
#include "cmd-test-image.h"
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
static uint16_t glob_str_to_uint16 (const char *pHexString);
static void cmd_engine_on_read_ready (int length, const unsigned char *pData);

static void cmd_engine_set_bg (const char *aParams);
static void cmd_engine_set_color (const char *aParams);
static void cmd_engine_set_scroll_start (const char *aParams);

static const char TheTestTextWithEscapeSequences[] PROGMEM =
  "Color tests. This is \033[30;40mblack on black\033[m.This is \033[31;40mred on b"
  "lack\033[m. This is \033[32;40mgreen on black\033[m. This is \033[33;40myellow o"
  "n black\033[m. This is \033[34;40mblue on black\033[m. This is \033[35;40mmagent"
  "a on black\033[m. This is \033[36;40mcyan on black\033[m. This is \033[37;40mwhi"
  "te on black\033[m. This is \033[33;40myello on black\033[m. This is \033[30;41mb"
  "lack on red\033[m. This is \033[30;42mblack on green\033[m. This is \033[30;43mb"
  "lack on yellow\033[m. This is \033[37;44mwhite on blue\033[m. This is \033[30;45"
  "mblack on magenta\033[m. This is \033[30;46mblack on cyan\033[m. This is \033[30"
  ";47mblack on white\033[m.";

static const char TheTestEscPositionManagement[] PROGMEM =
  "Hello world!!! This text will be erased in the next escape command, cool.\033[2J"
  "Initial text.\033[10;3HThis line starts at 10x3.\033[12;2fAnd this line starts a"
  "t 12x2.\033[30;10HSTART, 3U\033[3AUP, 6D\033[6BDOWN, 30B\033[30DBACK, 10F/3D\033"
  "[10C\033[3BFINAL!!!"
  "\033[40;4H\033[sTest menu:"
  "\033[u\033[s\033[1B1. Line 1 of the menu;"
  "\033[u\033[s\033[2B\033[30;47m2. Line 2 of the menu (selected);\033[0m"
  "\033[u\033[s\033[3B2. Line 3 of the menu;"
  "\033[u\033[s\033[4B2. Line 4 of the menu;"
  "\033[u\033[s\033[5B2. Line 5 of the menu;"
  "\033[u\033[7B\033[3DErase line test: \033[s1234567890\033[s123456789012345678901"
  "\033[u\033[K";

static const char TheLongTestText[] PROGMEM =
  "That's it! Now your data is in the Program Space. You can compile, link, and che"
  "ck the map file to verify that mydata is placed in the correct section. Now that"
  " your data resides in the Program Space, your code to access (read) the data wil"
  "l no longer work. The code that gets generated will retrieve the data that is lo"
  "cated at the address of the mydata array, plus offsets indexed by the i and j va"
  "riables. However, the final address that is calculated where to the retrieve the"
  " data points to the Data Space! Not the Program Space where the data is actually"
  " located. It is likely that you will be retrieving some garbage. The problem is "
  "that AVR GCC does not intrinsically know that the data resides in the Program Sp"
  "ace. Introduction. So you have some constant data and you're running out of room"
  " to store it? Many AVRs have limited amount of RAM in which to store data, but m"
  "ay have more Flash space available. The AVR is a Harvard architecture processor,"
  " where Flash is used for the program, RAM is used for data, and they each have s"
  "eparate address spaces. It is a challenge to get constant data to be stored in t"
  "he Program Space, and to retrieve that data to use it in the AVR application. Th"
  "e problem is exacerbated by the fact that the C Language was not designed for Ha"
  "rvard architectures, it was designed for Von Neumann architectures where code an"
  "d data exist in the same address space. This means that any compiler for a Harva"
  "rd architecture processor, like the AVR, has to use other means to operate with "
  "separate address spaces. Some compilers use non-standard C language keywords, or"
  " they extend the standard syntax in ways that are non-standard. The AVR toolset "
  "takes a different approach. GCC has a special keyword, __attribute__ that is use"
  "d to attach different attributes to things such as function declarations, variab"
  "les, and types. This keyword is followed by an attribute specification in double"
  " parentheses. In AVR GCC, there is a special attribute called progmem. This attr"
  "ibute is use on data declarations, and tells the compiler to place the data in t"
  "he Program Memory (Flash). Unfortunately, with GCC attributes, they affect only "
  "the declaration that they are attached to. So in this case, we successfully put "
  "the string_table variable, the array itself, in the Program Space. This DOES NOT"
  " put the actual strings themselves into Program Space. At this point, the string"
  "s are still in the Data Space, which is probably not what you want.";

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
    hw_uart_write_string_P (PSTR("> color xxxx - set text color\r\n"));
    hw_uart_write_string_P (PSTR("> bg xxxx - set background color\r\n"));

    hw_uart_write_string_P (PSTR("> cls - Clear screen\r\n"));
    hw_uart_write_string_P (PSTR("> reset - Reset LCD module\r\n"));
    hw_uart_write_string_P (PSTR("> on - Turn LCD module ON\r\n"));
    hw_uart_write_string_P (PSTR("> off - Turn LCD module OFF\r\n"));
    hw_uart_write_string_P (PSTR("> bs - Print <back-space> character\r\n"));
    hw_uart_write_string_P (PSTR("> tab - Print <tab> character\r\n"));
    hw_uart_write_string_P (PSTR("> ch - Print a single character\r\n"));
    hw_uart_write_string_P (PSTR("> line - Print a string with a new-line\r\n"));
    hw_uart_write_string_P (PSTR("> scroll <xxxx> - Scroll image\r\n"));
    hw_uart_write_string_P (PSTR("> timg - Load test image\r\n"));
    hw_uart_write_string_P (PSTR("> tstr - Show long string\r\n"));
    hw_uart_write_string_P (PSTR("> esc-pos - Show positioned test\r\n"));
    hw_uart_write_string_P (PSTR("> esc-color - Show colored strings\r\n"));
    hw_uart_write_string_P (PSTR("> tlimg - Load large test image\r\n"));
    hw_uart_write_string_P (PSTR("> l <IND> <1/0> - Turn ON/OFF the LEDs\r\n"));
    hw_uart_write_string_P (PSTR("> w <CMD> <DAT> - write <CMD> with <DAT> to I80\r\n"));
    hw_uart_write_string_P (PSTR("> r <CMD> <LEN> - read <LEN> bytes with <CMD> in I80\r\n"));
  }
#ifdef __linux__
  else if (!strcmp_P (aString, PSTR("quit")) || !strcmp_P (aString, PSTR("exit")))
  {
    /* EXIT command */
    main_request_exit ();
    start_uart_editor = 0;
  }
#endif /* __X86__ */
  else if (!strncmp_P (aString, PSTR("w "), 2))
  {
    /* WRITE command */
    cmd_engine_write (&aString[2]);
    start_uart_editor = 0;
  }
  else if (!strncmp_P (aString, PSTR("r "), 2))
  {
    /* READ command */
    cmd_engine_read (&aString[2]);
    start_uart_editor = 0;
  }
  else if (!strncmp_P (aString, PSTR("l "), 2))
  {
    cmd_engine_set_led (&aString[2]);
  }
  else if (!strcmp_P (aString, PSTR("reset")))
  {
    cmd_engine_reset ();
  }
  else if (!strcmp_P (aString, PSTR("on")))
  {
    hw_lcd_s95513_turn_on ();
  }
  else if (!strcmp_P (aString, PSTR("off")))
  {
    hw_lcd_s95513_turn_off ();
  }
  else if (!strcmp_P (aString, PSTR("timg")))
  {
    cmd_test_image ();
  }
  else if (!strcmp_P (aString, PSTR("tlimg")))
  {
    cmd_test_image_large ();
  }
  else if (!strcmp_P (aString, PSTR("cls")))
  {
    console_clear_screen ();
  }
  else if (!strncmp_P (aString, PSTR("bg "), 3))
  {
    cmd_engine_set_bg (&aString[3]);
  }
  else if (!strncmp_P (aString, PSTR("color "), 6))
  {
    cmd_engine_set_color (&aString[6]);
  }
  else if (!strncmp_P (aString, PSTR("scroll "), 7))
  {
    cmd_engine_set_scroll_start (&aString[7]);
  }
  else if (!strcmp_P (aString, PSTR("tstr")))
  {
    console_write_string_P (TheLongTestText);
  }
  else if (!strcmp_P (aString, PSTR("esc-color")))
  {
    console_write_string_P (TheTestTextWithEscapeSequences);
  }
  else if (!strcmp_P (aString, PSTR("esc-pos")))
  {
    console_write_string_P (TheTestEscPositionManagement);
  }
  else if (!strcmp_P (aString, PSTR("bs")))
  {
    console_write_string_P (PSTR ("\010"));
  }
  else if (!strcmp_P (aString, PSTR("tab")))
  {
    console_write_string_P (PSTR ("\t"));
  }
  else if (!strcmp_P (aString, PSTR("ch")))
  {
    static char str[] = "A";
    console_write_string (str);
    if (++str[0] > 'Z')
    {
      str[0] = 'A';
    }
  }
  else if (!strcmp_P (aString, PSTR("line")))
  {
    static uint8_t count = 0;

    char buffer[8];
    snprintf (buffer, 8, "%d", ++count);
    console_write_string_P (PSTR("This is a text line #"));
    console_write_string (buffer);
    console_write_string_P (PSTR("\r\n"));
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

void cmd_engine_set_bg (const char *aParams)
{
  if (4 == strlen (aParams))
  {
    const uint16_t param = glob_str_to_uint16 (aParams);
    console_set_bg_color (param);
  }
  else
  {
    hw_uart_write_string_P (PSTR("Wrong args, format: bg XXXX\r\n"));
  }
}

void cmd_engine_set_color (const char *aParams)
{
  if (4 == strlen (aParams))
  {
    const uint16_t param = glob_str_to_uint16 (aParams);
    console_set_color (param);
  }
  else
  {
    hw_uart_write_string_P (PSTR("Wrong args, format: color XXXX\r\n"));
  }
}

uint16_t glob_str_to_uint16 (const char *pHexString)
{
  return glob_get_byte (pHexString + 2) | (((uint16_t)glob_get_byte (pHexString)) << 8);
}

void cmd_engine_set_scroll_start (const char *aParams)
{
  if (4 == strlen (aParams))
  {
    hw_lcd_s95513_set_scroll_start (glob_str_to_uint16 (aParams));
  }
  else
  {
    hw_uart_write_string_P (PSTR("Wrong args, format: scroll <XXXX>\r\n"));
  }
}
