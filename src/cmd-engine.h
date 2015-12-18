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

#ifndef MCODE_CMD_ENGINE_H
#define MCODE_CMD_ENGINE_H

#include "mcode-config.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCODE_COMMAND_MODES
typedef enum {
  CmdModeNormal,
  CmdModeRoot,
  CmdModeUser
} CmdMode;
#endif /* MCODE_COMMAND_MODES */

void cmd_engine_init (void);
void cmd_engine_deinit (void);

void cmd_engine_start (void);

#ifdef MCODE_COMMAND_MODES
void cmd_engine_set_mode(CmdMode mode);
CmdMode cmd_engine_get_mode(void);
#endif /* MCODE_COMMAND_MODES */

void cmd_engine_system_help(void);
bool cmd_engine_system_command(const char *args, bool *startCmd);

#ifdef MCODE_TWI
void cmd_engine_twi_help(void);
bool cmd_engine_twi_read(const char *args);
bool cmd_engine_twi_write(const char *args);
#endif /* MCODE_TWI */

#ifdef MCODE_RTC
void cmd_engine_rtc_help(void);
bool cmd_engine_rtc_command(const char *args, bool *startCmd);
#endif /* MCODE_RTC */

#ifdef MCODE_TV
void cmd_engine_tv_init(void);
void cmd_engine_tv_help(void);
bool cmd_engine_tv_command(const char *args, bool *startCmd);
#endif /* MCODE_TV */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_CMD_ENGINE_H */
