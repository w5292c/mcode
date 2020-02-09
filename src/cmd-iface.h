/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Alexander Chumakov
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

#ifndef MCODE_CMD_IFACE_H
#define MCODE_CMD_IFACE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CMD_USED __attribute__((used))
#define CMD_SECTION __attribute__((section("command_section")))

#define CMD_ENTRY(name, help, handler, init_handler, level) \
  static const TCmdData name CMD_SECTION CMD_USED = { \
    help, \
    handler, \
    init_handler, \
    level \
  };

typedef void (*cmd_init_handler)(void);
typedef bool (*cmd_handler)(const char *args, bool *start_cmd);

typedef struct _TCmdData {
  const char *help;
  cmd_handler handler;
  cmd_init_handler init_handler;
  int level;
} TCmdData;

/**
 * Command section finish marker
 */
extern TCmdData __stop_command_section;
/**
 * Command section begin marker
 */
extern TCmdData __start_command_section;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_CMD_IFACE_H */
