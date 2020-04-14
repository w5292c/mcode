/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2020 Alexander Chumakov
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

#include "hw-uart.h"

#include "mtick.h"
#include "mstring.h"

#include <string.h>

#ifdef MCODE_UART2
#define MCODE_UART_READ_TIMEOUT (100) /*< 100ms */
#define UART2_READ_BUFFERS_COUNT (4)
#define MCODE_UART2_READ_BUFFER_LENGTH (512)

typedef enum {
  ELineReaderStateIdle,
  ELineReaderStateReadingLine,
  ELineReaderStateWaitingN,
} TReaderState;

typedef struct _TLineReaderState {
  uint16_t lineBufferLength[UART2_READ_BUFFERS_COUNT];
  char lineBuffer[UART2_READ_BUFFERS_COUNT][MCODE_UART2_READ_BUFFER_LENGTH];
  /**< The \c readIndex and \c writeIndex size must be enough
       for holding value of \c UART2_READ_BUFFERS_COUNT minus \c 1 */
  uint32_t readIndex : 2;
  uint32_t writeIndex : 2;
  uint32_t state : 4; /**< TReaderState */
  uint32_t ready : 4; /**< Ready indications for each read buffer */
} TLineReaderState;

volatile static TLineReaderState TheUart2State = {{0}};

static uint64_t TheLastCharTimestamp = 0;
static hw_uart_handler TheUart2Callback = NULL;
#endif /* MCODE_UART2 */

#ifndef __linux__
void hw_uart_deinit(void)
{
}
#endif /* __linux__ */

#ifdef MCODE_UART2
void hw_uart2_set_callback(hw_uart_handler cb)
{
  TheUart2Callback = cb;
}

void uart2_report_new_sample(void)
{
  bool timeout = false;
  /* Check if we have a ready line to handle */
  const uint8_t readyFlag = (1u << TheUart2State.readIndex);
  if (!(TheUart2State.ready & readyFlag)) {
    /* No ready data detected, check if we are 'idle' for too long (10ms) */
    if (!TheLastCharTimestamp || (mtick_count() - TheLastCharTimestamp) < 10 ||
        !TheUart2State.lineBufferLength[TheUart2State.readIndex]) {
      return;
    } else {
      timeout = true;
    }
  }

  /* Report the currently read line */
  (*TheUart2Callback)((const char *)TheUart2State.lineBuffer[TheUart2State.readIndex],
    TheUart2State.lineBufferLength[TheUart2State.readIndex]);

  /* Reset the reported buffer */
  memset((char *)TheUart2State.lineBuffer[TheUart2State.readIndex], 0, MCODE_UART2_READ_BUFFER_LENGTH);
  TheUart2State.lineBufferLength[TheUart2State.readIndex] = 0;
  /* Ready to read a line in the next buffer, reset the 'ready' flag for the current line */
  if (!timeout) {
    ++TheUart2State.readIndex;
  }
  TheUart2State.ready &= ~readyFlag;
}

void uart2_handle_new_sample(uint16_t data)
{
  TheLastCharTimestamp = mtick_count();
  size_t size = TheUart2State.lineBufferLength[TheUart2State.writeIndex];
  volatile char *const buffer = TheUart2State.lineBuffer[TheUart2State.writeIndex];
  switch (TheUart2State.state) {
  case ELineReaderStateIdle:
    size = 0;
    TheUart2State.lineBufferLength[TheUart2State.writeIndex] = 0;
    TheUart2State.state = ELineReaderStateReadingLine;
    /* Fall through to the next state */
  case ELineReaderStateReadingLine:
    if ('\r' == data) {
      /* We are reading the line, and get '\r' as another read character */
      TheUart2State.state = ELineReaderStateWaitingN;
      if (size < MCODE_UART2_READ_BUFFER_LENGTH - 1) {
        /* Check if we have enough space in the buffer for storing the new character */
        /* We reserve 1 byte for the end-of-line indication '\0' */
        buffer[size] = 0;
      } else {
        buffer[MCODE_UART2_READ_BUFFER_LENGTH - 1] = 0;
      }
    } else if (size < MCODE_UART2_READ_BUFFER_LENGTH - 1) {
      /* Check if we have enough space in the buffer for storing the new character */
      /* We reserve 1 byte for the end-of-line indication '\0' */
      buffer[size] = data;
    }
    ++TheUart2State.lineBufferLength[TheUart2State.writeIndex];
    break;
  case ELineReaderStateWaitingN:
    if ('\n' == data) {
      /* Got the right character, switch the buffers, read another line of text */
      /* assert: begin */
      if (TheUart2State.ready & (1u << TheUart2State.writeIndex)) {
        mprintstrln("E: not ready");
      }
      /* assert: end */
      TheUart2State.ready |= (1u << TheUart2State.writeIndex);
      TheUart2State.state = ELineReaderStateIdle;
      ++TheUart2State.writeIndex;
    } else {
      /* Wrong new-line sequence received, reset to idle state, report error */
      mprintstr("E:");
      mprint_uint16(data, true);
      mprint(MStringNewLine);
      TheUart2State.state = ELineReaderStateIdle;
    }
    break;
  }
}
#endif /* MCODE_UART2 */
