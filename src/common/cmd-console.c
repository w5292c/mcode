/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2020 Alexander Chumakov
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
#include "hw-lcd.h"
#include "console.h"
#include "mglobal.h"
#include "mstring.h"

#include <string.h>

static void cmd_engine_set_bg (const char *aParams);
static void cmd_engine_set_color (const char *aParams);
static void cmd_engine_set_scroll_start (const char *aParams);

#ifdef MCODE_TEST_STRINGS
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
  "\033[u\033[KZ";

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
#endif /* MCODE_TEST_STRINGS */

void cmd_engine_console_help(void)
{
  mprintstrln(PSTR("> bs - Print <back-space> character"));
  mprintstrln(PSTR("> tab - Print <tab> character"));
  mprintstrln(PSTR("> ch - Print a single character"));
  mprintstrln(PSTR("> line - Print a string with a new-line"));
  mprintstrln(PSTR("> scroll <xxxx> - Scroll image"));
#ifdef MCODE_TEST_STRINGS
  mprintstrln(PSTR("> tstr - Show long string"));
  mprintstrln(PSTR("> esc-pos - Show positioned test"));
  mprintstrln(PSTR("> esc-color - Show colored strings"));
#endif /* MCODE_TEST_STRINGS */
  mprintstrln(PSTR("> color xxxx - set text color"));
  mprintstrln(PSTR("> bg xxxx - set background color"));
  mprintstrln(PSTR("> cls - Clear screen"));
}

bool cmd_engine_console_command(const char *command, bool *startCmd)
{
  if (!strcmp_P(command, PSTR("cls"))) {
    console_clear_screen();
    return true;
  } else if (!strncmp_P(command, PSTR("bg "), 3)) {
    cmd_engine_set_bg(&command[3]);
    return true;
  } else if (!strncmp_P(command, PSTR("color "), 6)) {
    cmd_engine_set_color(&command[6]);
    return true;
  } else if (!strncmp_P(command, PSTR("scroll "), 7)) {
    cmd_engine_set_scroll_start(&command[7]);
    return true;
#ifdef MCODE_TEST_STRINGS
  } else if (!strcmp_P(command, PSTR("tstr"))) {
    console_write_string_P(TheLongTestText);
    console_write_string_P(TheLongTestText);
    return true;
  } else if (!strcmp_P(command, PSTR("esc-color"))) {
    console_write_string_P(TheTestTextWithEscapeSequences);
    return true;
  } else if (!strcmp_P(command, PSTR("esc-pos"))) {
    console_write_string_P(TheTestEscPositionManagement);
    return true;
#endif /* MCODE_TEST_STRINGS */
  } else if (!strcmp_P(command, PSTR("bs"))) {
    console_write_string_P(PSTR ("\010"));
    return true;
  } else if (!strcmp_P(command, PSTR("tab"))) {
    console_write_string_P(PSTR ("\t"));
    return true;
  } else if (!strcmp_P(command, PSTR("ch"))) {
    static char str[] = "A";
    console_write_string(str);
    if (++str[0] > 'Z') {
      str[0] = 'A';
    }
    return true;
  } else if (!strcmp_P(command, PSTR("line"))) {
    static uint8_t count = 0;

    console_write_string_P(PSTR("This is a text line #"));
    console_write_uint32(count++, false);
    console_write_string_P(PSTR("\r\n"));
    return true;
  }

  return false;
}

void cmd_engine_set_bg (const char *aParams)
{
  if (4 == strlen (aParams)) {
    const uint16_t param = glob_str_to_uint16 (aParams);
    console_set_bg_color (param);
  } else {
    merror(MStringWrongArgument);
  }
}

void cmd_engine_set_color (const char *aParams)
{
  if (4 == strlen (aParams)) {
    const uint16_t param = glob_str_to_uint16 (aParams);
    console_set_color (param);
  } else {
    merror(MStringWrongArgument);
  }
}

void cmd_engine_set_scroll_start(const char *args)
{
  if (4 == strlen (args)) {
    lcd_set_scroll_start(glob_str_to_uint16(args));
  } else {
    merror(MStringWrongArgument);
  }
}
