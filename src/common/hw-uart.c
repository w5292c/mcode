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
  ELineReaderStateWaitingSpace,
  ELineReaderStateError,
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
  const uint8_t ready_flag = (1u << TheUart2State.readIndex);
  if (!(TheUart2State.ready & ready_flag)) {
    /* No ready data to report */
    return;
  }

  /* Report the currently read line */
  (*TheUart2Callback)((const char *)TheUart2State.lineBuffer[TheUart2State.readIndex],
    TheUart2State.lineBufferLength[TheUart2State.readIndex]);

  /* Reset the reported buffer */
  memset((char *)TheUart2State.lineBuffer[TheUart2State.readIndex], 0, MCODE_UART2_READ_BUFFER_LENGTH);
  TheUart2State.lineBufferLength[TheUart2State.readIndex] = 0;

  /* Ready to read a line in the next buffer,
     reset the 'ready' flag for the current buffer and move to the next buffer */
  ++TheUart2State.readIndex;
  TheUart2State.ready &= ~ready_flag;

  /* Check if the are in an ERROR state, reset it, as we have an empty buffer */
  if (ELineReaderStateError == TheUart2State.state) {
    TheUart2State.state = ELineReaderStateIdle;
  }
}

void uart2_handle_new_sample(uint16_t data)
{
  bool finalize;
  bool skip_char;
  unsigned int flag;

  /* Input filter */
  if (data >= 128 || !data || TheUart2State.state == ELineReaderStateError) {
    return;
  }

  finalize = false;
  skip_char = false;
  switch (TheUart2State.state) {
  case ELineReaderStateIdle:
    flag = (1u << TheUart2State.writeIndex);
    if ((TheUart2State.ready & flag) != 0) {
      /* The current buffer is not ready, cannot exit the IDLE state, move to ERROR state */
      TheUart2State.state = ELineReaderStateError;
      return;
    }
    if ('>' == data) {
      TheUart2State.state = ELineReaderStateWaitingSpace;
    } else if ('\r' == data) {
      /* Do not skip the '\r' for now, as the next char might not be '\n' */
      TheUart2State.state = ELineReaderStateWaitingN;
    } else {
      TheUart2State.state = ELineReaderStateReadingLine;
    }
    break;
  case ELineReaderStateReadingLine:
    if ('\r' == data) {
      TheUart2State.state = ELineReaderStateWaitingN;
    }
    break;
  case ELineReaderStateWaitingN:
    if ('\n' == data) {
      skip_char = true;
      finalize = true;
    } else {
      TheUart2State.state = ELineReaderStateReadingLine;
    }
    break;
  case ELineReaderStateWaitingSpace:
    if (' ' == data) {
      finalize = true;
    } else if ('\r' == data) {
      TheUart2State.state = ELineReaderStateWaitingN;
    } else {
      TheUart2State.state = ELineReaderStateReadingLine;
    }
    break;
  }

  if (!skip_char) {
    volatile uint16_t *const buffer_length = TheUart2State.lineBufferLength + TheUart2State.writeIndex;
    if (*buffer_length < MCODE_UART2_READ_BUFFER_LENGTH - 1) {
      TheUart2State.lineBuffer[TheUart2State.writeIndex][*buffer_length] = data;
      ++*buffer_length;
    }
  }
  if (finalize) {
    /* Check if the next-to-the-last character is '\r', remove it if it is */
    volatile uint16_t *const buffer_length = TheUart2State.lineBufferLength + TheUart2State.writeIndex;
    if ('\r' == TheUart2State.lineBuffer[TheUart2State.writeIndex][*buffer_length - 1]) {
      TheUart2State.lineBuffer[TheUart2State.writeIndex][*buffer_length - 1] = 0;
      --*buffer_length;
    }

    flag = (1u << TheUart2State.writeIndex);
    TheUart2State.ready |= flag;
    ++TheUart2State.writeIndex;
    TheUart2State.state = ELineReaderStateIdle;
  }
}
#endif /* MCODE_UART2 */
