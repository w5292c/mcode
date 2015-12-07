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

#include "pwm.h"
#include "main.h"
#include "mtick.h"
#include "utils.h"
#include "system.h"
#include "hw-i80.h"
#include "hw-lcd.h"
#include "mglobal.h"
#include "console.h"
#include "hw-leds.h"
#include "hw-uart.h"
#include "mcode-config.h"
#include "cmd-test-image.h"
#include "line-editor-uart.h"
#include "persistent-store.h"

#include <string.h>

#ifdef MCODE_HW_I80_ENABLED
static void cmd_engine_read (const char *aCommand);
static void cmd_engine_write (const char *aCommand);
#endif /* MCODE_HW_I80_ENABLED */
#ifdef MCODE_PWM
static void cmd_engine_pwm(const char *cmd);
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
static void cmd_engine_set_led(const char *cmd);
#endif /* MCODE_LEDS */
static void cmd_engine_on_cmd_ready (const char *aString);
#ifdef MCODE_CONSOLE_ENABLED
static void cmd_engine_set_bg (const char *aParams);
static void cmd_engine_set_color (const char *aParams);
static void cmd_engine_set_scroll_start (const char *aParams);
#endif /* MCODE_CONSOLE_ENABLED */
#ifdef MCODE_SECURITY
#include "sha.h"

static void cmd_engine_sha256(const char *aParams);
#endif /* MCODE_SECURITY */
#ifdef MCODE_COMMAND_MODES
typedef enum {
  CommandEngineStateCmd,
  CommandEngineStateArgs,
  CommandEngineStatePass,
} CommandEngineState;

static uint8_t TheMode;
static uint32_t TheSuperUserTimeout;
static uint8_t TheCommandEngineState = CommandEngineStateCmd;
static uint8_t TheCommandEngineStateRequest;
static void cmd_engine_mtick(void);
static void cmd_engine_passwd(void);
static void cmd_engine_set_cmd_mode(const char *params);
static void cmd_engine_handle_args(const char *args);
static void cmd_engine_handle_pass(const char *pass);
#endif /* MCODE_COMMAND_MODES */

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

void cmd_engine_init (void)
{
  line_editor_uart_init();
  line_editor_uart_set_callback(cmd_engine_on_cmd_ready);
#ifdef MCODE_COMMAND_MODES
  TheMode = CmdModeNormal;
  TheSuperUserTimeout = 0;
  mtick_add(cmd_engine_mtick);
#endif /* MCODE_COMMAND_MODES */
}

void cmd_engine_deinit (void)
{
  line_editor_uart_deinit();
}

void cmd_engine_start (void)
{
  line_editor_uart_start();
}

void cmd_engine_on_cmd_ready (const char *aString)
{
#ifdef MCODE_COMMAND_MODES
  switch (TheCommandEngineState) {
  case CommandEngineStateCmd:
    break;
  case CommandEngineStateArgs:
    cmd_engine_handle_args(aString);
    return;
  case CommandEngineStatePass:
    cmd_engine_handle_pass(aString);
    return;
  default:
    hw_uart_write_string_P(PSTR("Error: wrong engine state: "));
    hw_uart_write_uint(TheCommandEngineState);
    hw_uart_write_string_P(PSTR("\r\n"));
    return;
  }
#endif /* MCODE_COMMAND_MODES */

  int start_uart_editor = 1;

  if (!strcmp_P (aString, PSTR("help")))
  {
    /* HELP command */
    hw_uart_write_string_P(PSTR("Supported cmds:\r\n"));
#if __linux__ == 1
    hw_uart_write_string_P(PSTR("> exit/quit - exit\r\n"));
#endif /* __linux__ == 1 */
    hw_uart_write_string_P(PSTR("> ut - Show uptime\r\n"));
    hw_uart_write_string_P(PSTR("> reboot - Initiate system reboot\r\n"));
#ifdef MCODE_CONSOLE_ENABLED
    hw_uart_write_string_P(PSTR("> color xxxx - set text color\r\n"));
    hw_uart_write_string_P(PSTR("> bg xxxx - set background color\r\n"));
    hw_uart_write_string_P(PSTR("> cls - Clear screen\r\n"));
#endif /* MCODE_CONSOLE_ENABLED */
#ifdef MCODE_LCD
    hw_uart_write_string_P(PSTR("> reset - Reset LCD module\r\n"));
    hw_uart_write_string_P(PSTR("> on - Turn LCD module ON\r\n"));
    hw_uart_write_string_P(PSTR("> off - Turn LCD module OFF\r\n"));
    hw_uart_write_string_P(PSTR("> lcd-id Read the LCD ID\r\n"));
#endif /* MCODE_LCD */
#ifdef MCODE_CONSOLE_ENABLED
    hw_uart_write_string_P(PSTR("> bs - Print <back-space> character\r\n"));
    hw_uart_write_string_P(PSTR("> tab - Print <tab> character\r\n"));
    hw_uart_write_string_P(PSTR("> ch - Print a single character\r\n"));
    hw_uart_write_string_P(PSTR("> line - Print a string with a new-line\r\n"));
    hw_uart_write_string_P(PSTR("> scroll <xxxx> - Scroll image\r\n"));
    hw_uart_write_string_P(PSTR("> timg - Load test image\r\n"));
    hw_uart_write_string_P(PSTR("> tstr - Show long string\r\n"));
    hw_uart_write_string_P(PSTR("> esc-pos - Show positioned test\r\n"));
    hw_uart_write_string_P(PSTR("> esc-color - Show colored strings\r\n"));
    hw_uart_write_string_P(PSTR("> tlimg - Load large test image\r\n"));
#endif /* MCODE_CONSOLE_ENABLED */
#ifdef MCODE_PWM
    hw_uart_write_string_P(PSTR("> pwm <ind> <value> - Set PWM value\r\n"));
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
    hw_uart_write_string_P(PSTR("> led <IND> <1/0> - Turn ON/OFF the LEDs\r\n"));
#endif /* MCODE_LEDS */
#ifdef MCODE_HW_I80_ENABLED
    hw_uart_write_string_P(PSTR("> w <CMD> <DAT> - write <CMD> with <DAT> to I80\r\n"));
    hw_uart_write_string_P(PSTR("> r <CMD> <LEN> - read <LEN> bytes with <CMD> in I80\r\n"));
#endif /* MCODE_HW_I80_ENABLED */
#ifdef MCODE_SECURITY
    hw_uart_write_string_P(PSTR("> sha256 <DATA> - calculate SHA256 hash\r\n"));
#endif /* MCODE_SECURITY */
#ifdef MCODE_COMMAND_MODES
    hw_uart_write_string_P(PSTR("> su [MODE(1|2|3)] - Set the command engine mode\r\n"));
    hw_uart_write_string_P(PSTR("> passwd - change the device password\r\n"));
#endif /* MCODE_COMMAND_MODES */
  }
#ifdef __linux__
  else if (!strcmp_P(aString, PSTR("quit")) || !strcmp_P(aString, PSTR("exit"))) {
    /* EXIT command */
    main_request_exit();
    start_uart_editor = 0;
  }
#endif /* __X86__ */
  else if (!strcmp_P(aString, PSTR("ut"))) {
    hw_uart_write_string_P(PSTR("Uptime: 0x"));
    hw_uart_write_uint64(mtick_count(), true);
    hw_uart_write_string_P(PSTR("\r\n"));
  } else if (!strcmp_P(aString, PSTR("reboot"))) {
    hw_uart_write_string_P(PSTR("\r\n"));
    reboot();
  }
#ifdef MCODE_HW_I80_ENABLED
  else if (!strncmp_P(aString, PSTR("w "), 2)) {
    /* WRITE command */
    cmd_engine_write(&aString[2]);
  } else if (!strncmp_P(aString, PSTR("r "), 2)) {
    /* READ command */
    cmd_engine_read(&aString[2]);
  }
#endif /* MCODE_HW_I80_ENABLED */
#ifdef MCODE_PWM
  else if (!strncmp_P(aString, PSTR("pwm "), 4)) {
    cmd_engine_pwm(&aString[4]);
  }
#endif /* MCODE_PWM */
#ifdef MCODE_LEDS
  else if (!strncmp_P(aString, PSTR("led "), 4)) {
    cmd_engine_set_led(&aString[4]);
  }
#endif /* MCODE_LEDS */
#ifdef MCODE_LCD
  else if (!strcmp_P(aString, PSTR("reset"))) {
    lcd_reset();
  } else if (!strcmp_P(aString, PSTR("on"))) {
    lcd_turn(true);
    lcd_set_bl(true);
  } else if (!strcmp_P(aString, PSTR("off"))) {
    lcd_set_bl(false);
    lcd_turn(false);
  } else if (!strcmp_P(aString, PSTR("lcd-id"))) {
    const uint32_t id = lcd_read_id();
    hw_uart_write_string("LCD ID: 0x");
    hw_uart_write_uint32(id, false);
    hw_uart_write_string("\r\n");
  }
#endif /* MCODE_LCD */
#ifdef MCODE_TEST_IMAGES
  else if (!strcmp_P(aString, PSTR("timg"))) {
    cmd_test_image();
  } else if (!strcmp_P(aString, PSTR("tlimg"))) {
    cmd_test_image_large();
  }
#endif /* MCODE_TEST_IMAGES */
#ifdef MCODE_CONSOLE_ENABLED
  else if (!strcmp_P(aString, PSTR("cls"))) {
    console_clear_screen();
  } else if (!strncmp_P(aString, PSTR("bg "), 3)) {
    cmd_engine_set_bg(&aString[3]);
  } else if (!strncmp_P(aString, PSTR("color "), 6)) {
    cmd_engine_set_color(&aString[6]);
  } else if (!strncmp_P(aString, PSTR("scroll "), 7)) {
    cmd_engine_set_scroll_start(&aString[7]);
  } else if (!strcmp_P(aString, PSTR("tstr"))) {
    console_write_string_P(TheLongTestText);
    console_write_string_P(TheLongTestText);
  } else if (!strcmp_P(aString, PSTR("esc-color"))) {
    console_write_string_P(TheTestTextWithEscapeSequences);
  } else if (!strcmp_P(aString, PSTR("esc-pos"))) {
    console_write_string_P(TheTestEscPositionManagement);
  } else if (!strcmp_P(aString, PSTR("bs"))) {
    console_write_string_P(PSTR ("\010"));
  } else if (!strcmp_P(aString, PSTR("tab"))) {
    console_write_string_P(PSTR ("\t"));
  } else if (!strcmp_P(aString, PSTR("ch"))) {
    static char str[] = "A";
    console_write_string(str);
    if (++str[0] > 'Z') {
      str[0] = 'A';
    }
  } else if (!strcmp_P(aString, PSTR("line"))) {
    static uint8_t count = 0;

    console_write_string_P(PSTR("This is a text line #"));
    console_write_uint32(count++, false);
    console_write_string_P(PSTR("\r\n"));
  }
#endif /* MCODE_CONSOLE_ENABLED */
#ifdef MCODE_SECURITY
  else if (!strncmp_P(aString, PSTR("sha256 "), 7)) {
    cmd_engine_sha256(&aString[7]);
  }
#endif /* MCODE_SECURITY */
#ifdef MCODE_COMMAND_MODES
  else if (!strncmp_P(aString, PSTR("su "), 3)) {
    cmd_engine_set_cmd_mode(&aString[3]);
  } else if (!strcmp_P(aString, PSTR("passwd"))) {
    cmd_engine_passwd();
  }
#endif /* MCODE_COMMAND_MODES */
  else if (*aString) {
    /* got unrecognized non-empty command */
    hw_uart_write_string_P(PSTR("ENGINE: unrecognized command. Type 'help'.\r\n"));
  }

#ifdef MCODE_COMMAND_MODES
  if (TheCommandEngineState == CommandEngineStateCmd && start_uart_editor) {
    line_editor_uart_start();
  }
#else /* MCODE_COMMAND_MODES */
  if (start_uart_editor) {
    line_editor_uart_start();
  }
#endif /* MCODE_COMMAND_MODES */
}

#ifdef MCODE_HW_I80_ENABLED
void cmd_engine_read(const char *aCommand)
{
  uint8_t buffer[16];
  uint8_t command = 0;
  uint8_t dataLength = 0;

  memset(buffer, 0, 16);
  /* first, retrieve the command code */
  int value = 0;
  aCommand = string_skip_whitespace(aCommand);
  aCommand = string_next_number(aCommand, &value);
  command = (uint8_t)value;
  aCommand = string_skip_whitespace(aCommand);

  if (aCommand) {
    value = 0;
    string_next_number(aCommand, &value);
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

    uint8_t i;
    for (i = 0; i < dataLength; ++i) {
      hw_uart_write_uint(buffer[i]);
      if (i != dataLength - 1) {
        hw_uart_write_string_P(PSTR(" "));
      }
    }
    hw_uart_write_string_P (PSTR("\r\n"));
  } else {
    hw_uart_write_string_P(PSTR("Wrong args, format: r <command> <number-of-byte-to-read>\r\n"));
  }
}

void cmd_engine_write(const char *aCommand)
{
  int success = 1;
  int dataLength = 0;
  unsigned char command;
#define MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH (32)
  unsigned char buffer[MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH];

  memset(buffer, 0, MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH);

  /* parse arguments */
  const unsigned char ch0 = aCommand[0];
  const unsigned char ch1 = aCommand[1];
  const unsigned char ch2 = aCommand[2];

  if (ch0 && ch1 && char_is_hex(ch0) && char_is_hex(ch1) && (' ' == ch2)) {
    command = glob_get_byte (aCommand);
    aCommand += 3;

    while (*aCommand) {
      volatile uint8_t chH = aCommand[0];
      volatile uint8_t chL = aCommand[1];
      if (chH && chL && char_is_hex(chH) && char_is_hex(chL) && (dataLength < MCODE_CMD_ENGINE_WRITE_BUFFER_LENGTH - 1)) {
        buffer[dataLength++] = (glob_ch_to_val(chH) << 4) | glob_ch_to_val(chL);
        aCommand += 2;
      } else {
        success = 0;
        break;
      }
    }
  } else {
    success = 0;
  }

  if (success) {
    /* pass the request to the I80 bus */
    hw_i80_write(command, dataLength, buffer);
  } else {
    hw_uart_write_string_P (PSTR("Wrong args, format: W CC X1[X2[X3..XA]]\r\n"));
  }
}
#endif /* MCODE_HW_I80_ENABLED */

#ifdef MCODE_PWM
void cmd_engine_pwm(const char *cmd)
{
  int index = -1;
  int value = -1;
  const char *const secondNumber = string_skip_whitespace(string_next_number(string_skip_whitespace(cmd), &index));
  if (secondNumber) {
    string_next_number(secondNumber, &value);
  }

  if (index < 0 || index > 2 || value < 0 || value > 255) {
    hw_uart_write_string_P(PSTR("Wrong args, format: pwm <index> <value>\r\n"));
    hw_uart_write_string_P(PSTR("Possible indexes: [0..2], values: [0..255]\r\n"));
    return;
  }

  pwm_set(index, value);
}
#endif /* MCODE_PWM */

#ifdef MCODE_LEDS
/**
 * Set-LED command handler
 * @param[in] aCommand - The command parameters
 * @note The command parameters should go in the following format: "X Y";
 * X may be either 0 or 1 (LED index) and Y may be either 0 or 1 (OFF or ON).
 */
void cmd_engine_set_led(const char *cmd)
{
  int index = -1;
  int value = -1;
  const char *const secondNumber = string_skip_whitespace(string_next_number(string_skip_whitespace(cmd), &index));
  if (secondNumber) {
    string_next_number(secondNumber, &value);
  }
  if (index >= 0) {
    if (value >= 0) {
      mcode_hw_leds_set(index, value);
    } else {
      const uint8_t on = mcode_hw_leds_get(index);
      hw_uart_write_string_P(PSTR("LED"));
      hw_uart_write_uint(index);
      hw_uart_write_string_P(PSTR(", value: "));
      hw_uart_write_uint(on);
      hw_uart_write_string_P(PSTR("\r\n"));
    }
  } else {
    hw_uart_write_string_P (PSTR("Wrong args, format: L I 0/1\r\n> I - the LED index [0..1]\r\n> 0/1 - turm OFF/ON\r\n"));
  }
}
#endif /* MCODE_LEDS */

#ifdef MCODE_CONSOLE_ENABLED
void cmd_engine_set_bg (const char *aParams)
{
  if (4 == strlen (aParams)) {
    const uint16_t param = glob_str_to_uint16 (aParams);
    console_set_bg_color (param);
  } else {
    hw_uart_write_string_P (PSTR("Wrong args, format: bg XXXX\r\n"));
  }
}

void cmd_engine_set_color (const char *aParams)
{
  if (4 == strlen (aParams)) {
    const uint16_t param = glob_str_to_uint16 (aParams);
    console_set_color (param);
  } else {
    hw_uart_write_string_P (PSTR("Wrong args, format: color XXXX\r\n"));
  }
}

void cmd_engine_set_scroll_start (const char *aParams)
{
  if (4 == strlen (aParams)) {
    lcd_set_scroll_start(glob_str_to_uint16 (aParams));
  } else {
    hw_uart_write_string_P (PSTR("Wrong args, format: scroll <XXXX>\r\n"));
  }
}
#endif /* MCODE_CONSOLE_ENABLED */

#ifdef MCODE_SECURITY
void cmd_engine_sha256(const char *aParams)
{
  const uint16_t n = strlen(aParams);
  hw_uart_write_string_P(PSTR("Calculating sha256 hash, string: '"));
  hw_uart_write_string(aParams);
  hw_uart_write_string_P(PSTR("', length: 0x"));
  hw_uart_write_uint(n);
  hw_uart_write_string_P(PSTR("\r\n"));

  uint8_t byteResultHash[SHA256_DIGEST_LENGTH];
  SHA256((const unsigned char *)aParams, n, byteResultHash);

  uint8_t i;
  uint8_t *ptr = byteResultHash;
  for (i = 0; i < SHA256_DIGEST_LENGTH; i += 2, ptr += 2) {
    uint16_t data = ((*ptr) << 8) | (*(ptr + 1) << 0);
    hw_uart_write_uint16(data, false);
  }
  hw_uart_write_string_P(PSTR("\r\n"));
}
#endif /* MCODE_SECURITY */

#ifdef MCODE_COMMAND_MODES
typedef enum {
  ModesStateIdle,
  ModesStateEnterPass,
  ModesStateChangePassEnterCurrent,
  ModesStateChangePassEnterNew,
  ModesStateChangePassConfirmNew
} ModesState;

static uint8_t TheModesState;

void cmd_engine_set_mode(CmdMode mode)
{
  switch (mode) {
  case CmdModeRoot:
    /* super-user mode timeout: 5mins */
    TheSuperUserTimeout = 300000UL;
    /* fall through */
  case CmdModeNormal:
  case CmdModeUser:
    TheMode = mode;
    break;
  default:
    hw_uart_write_string_P(PSTR("Error: 'cmd_engine_set_mode' - wrong mode: 0x"));
    hw_uart_write_uint(mode);
    hw_uart_write_string_P(PSTR("\r\n"));
    break;
  }
}

CmdMode cmd_engine_get_mode(void)
{
  return (CmdMode)TheMode;
}

void cmd_engine_set_cmd_mode(const char *params)
{
  int value = 0;
  const char *next = string_next_number(string_skip_whitespace(params), &value);
  if (next && !*next && value > 0 && value < 4) {
    TheCommandEngineStateRequest = (CmdMode)(value - 1);
    hw_uart_write_string_P(PSTR("Enter password: "));
    TheCommandEngineState = CommandEngineStatePass;
    TheModesState = ModesStateEnterPass;
    line_editor_set_echo(false);
  } else {
    hw_uart_write_string_P(PSTR("Error: wrong arguments for 'su'.\r\nFormat: $ su [1/2/3]\r\n"));
  }
}

void cmd_engine_handle_args(const char *args)
{
  hw_uart_write_string_P(PSTR("Got args: '"));
  hw_uart_write_string(args);
  hw_uart_write_string_P(PSTR("'\r\n"));
  TheCommandEngineState = CommandEngineStateCmd;
  line_editor_uart_start();
}

void cmd_engine_handle_pass(const char *pass)
{
  const uint16_t n = strlen(pass);
  uint8_t hash[SHA256_DIGEST_LENGTH];
  uint8_t passHash[SHA256_DIGEST_LENGTH];
  SHA256((const uint8_t *)pass, n, passHash);
  if (ModesStateEnterPass == TheModesState ||
      ModesStateChangePassEnterCurrent == TheModesState) {
    persist_store_load(PersistStoreIdHash, hash, SHA256_DIGEST_LENGTH);

    if (!memcmp(hash, passHash, SHA256_DIGEST_LENGTH)) {
      cmd_engine_set_mode((CmdMode)TheCommandEngineStateRequest);
    } else {
      hw_uart_write_string_P(PSTR("Error: something wrong\r\n"));

      line_editor_set_echo(true);
      TheModesState = ModesStateIdle;
      TheCommandEngineState = CommandEngineStateCmd;
      line_editor_uart_start();
    }

    if (ModesStateEnterPass == TheModesState) {
      line_editor_set_echo(true);
      TheModesState = ModesStateIdle;
      TheCommandEngineState = CommandEngineStateCmd;
      line_editor_uart_start();
    } else if (ModesStateChangePassEnterCurrent == TheModesState) {
      hw_uart_write_string_P(PSTR("Enter new password: "));
      TheModesState = ModesStateChangePassEnterNew;
    }
  } else if (ModesStateChangePassEnterNew == TheModesState) {
    persist_store_save(PersistStoreIdNewHash, passHash, SHA256_DIGEST_LENGTH);
    hw_uart_write_string_P(PSTR("Confirm new password: "));
    TheModesState = ModesStateChangePassConfirmNew;
  } else if (ModesStateChangePassConfirmNew == TheModesState) {
    persist_store_load(PersistStoreIdNewHash, hash, SHA256_DIGEST_LENGTH);

    if (!memcmp(hash, passHash, SHA256_DIGEST_LENGTH)) {
      persist_store_save(PersistStoreIdHash, passHash, SHA256_DIGEST_LENGTH);
    } else {
      hw_uart_write_string_P(PSTR("Error: password missmatch\r\n"));
    }

    line_editor_set_echo(true);
    TheModesState = ModesStateIdle;
    TheCommandEngineState = CommandEngineStateCmd;
    line_editor_uart_start();
  }
}

void cmd_engine_mtick(void)
{
  if (TheSuperUserTimeout && !(--TheSuperUserTimeout)) {
    if (CmdModeRoot == TheMode) {
      cmd_engine_set_mode(CmdModeNormal);
    }
  }
}

void cmd_engine_passwd(void)
{
  hw_uart_write_string_P(PSTR("Enter current password: "));
  TheCommandEngineState = CommandEngineStatePass;
  TheModesState = ModesStateChangePassEnterCurrent;
  line_editor_set_echo(false);
}

#endif /* MCODE_COMMAND_MODES */
