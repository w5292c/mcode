/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Alexander Chumakov
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

#include "hw-twi.h"

#include "hw-uart.h"
#include "mstring.h"
#include "scheduler.h"

#include <util/twi.h>
#include <avr/interrupt.h>

typedef enum {
  /* system states */
  ETwiStateNull = 0,
  ETwiStateIdle,
  ETwiStateCancelling,
  /* TWI reading states */
  ETwiStateRdSendStart,
  ETwiStateRdSendSlvAddr,
  ETwiStateRdReadingBytes,
  ETwiStateRdReadingBytesLast,
  ETwiStateRdDone,
  ETwiStateRdDoneError,
  /* TWI writing states */
  ETwiStateWrSendStart,
  ETwiStateWrSendSlvAddr,
  ETwiStateWrWritingData,
  ETwiStateWrDone,
  ETwiStateWrDoneError
} TwiCState;

#ifndef READ_BUFFER_LENGTH
#define READ_BUFFER_LENGTH (32)
#else /* READ_BUFFER_LENGTH */
#warning "READ_BUFFER_LENGTH already defined"
#endif /* READ_BUFFER_LENGTH */

static mcode_read_ready TheReadCallback;
static mcode_done TheWriteCallback;

static uint8_t volatile TheTwiIndex = 0;
static uint8_t volatile TheTwiState = ETwiStateNull;
/* The cached client request */
static const uint8_t volatile *TheWriteBuffer;
static uint8_t volatile TheReadBuffer[READ_BUFFER_LENGTH];
static uint8_t volatile TheRequestAddress;
static uint8_t volatile TheRequestLength;

/**
 * The scheduler tick
 * @return TRUE if more work is already available
 */
static void hw_twi_sched_tick(void);
/**
 * Send 'start' TWI protocol condition
 */
static inline void hw_twi_send_start(void);

void twi_init(void)
{
  TheTwiState = ETwiStateIdle;
  mcode_scheduler_add(hw_twi_sched_tick);

  TWBR = 0x0CU;
  TWDR = 0xFFU;
  TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
  PORTC |= (1<<PC0) | (1<<PC1);
}

void twi_deinit(void)
{
}

void twi_recv(uint8_t addr, uint8_t length, mcode_read_ready callback)
{
  if (ETwiStateIdle == TheTwiState) {
    TheRequestAddress = (addr & 0xFEU);
    TheRequestLength = length;

    TheReadCallback = callback;
    TheTwiState = ETwiStateRdSendStart;
    hw_twi_send_start();
  } else {
    merror(MStringInternalError);
    if (callback) {
      (*callback)(false, 0, NULL);
    }
  }
}

void twi_send(uint8_t addr, uint8_t length, const uint8_t *data, mcode_done callback)
{
  if (ETwiStateIdle == TheTwiState) {
    TheRequestAddress = (addr & 0xFEU);
    TheRequestLength = length;
    TheWriteBuffer = data;

    TheWriteCallback = callback;
    TheTwiState = ETwiStateWrSendStart;
    hw_twi_send_start();
  } else {
    if (callback) {
      (*callback)(false);
    }
  }
}

static void hw_twi_sched_tick(void)
{
  switch (TheTwiState)
  {
  case ETwiStateWrDone:
    TheTwiState = ETwiStateIdle;
    if (TheWriteCallback) {
      (*TheWriteCallback)(true);
    }
    break;
  case ETwiStateWrDoneError:
    TheTwiState = ETwiStateIdle;
    if (TheWriteCallback) {
      (*TheWriteCallback)(false);
    }
    break;
  case ETwiStateRdDone:
    TheTwiState = ETwiStateIdle;
    if (TheReadCallback) {
      (*TheReadCallback)(true, TheRequestLength, (const uint8_t *)TheReadBuffer);
    }
    break;
  case ETwiStateRdDoneError:
    TheTwiState = ETwiStateIdle;
    if (TheReadCallback) {
      (*TheReadCallback)(false, 0, NULL);
    }
    break;
  default:
    break;
  }
}

static inline void hw_twi_send_start(void)
{
  /* send the START condition */
  TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN)) | (1<<TWIE);
}

/**
 * Handle the event that the START condition is sent
 */
static inline void hw_twi_handle_start_transmitted(void)
{
  switch (TheTwiState) {
  case ETwiStateWrSendStart:
  case ETwiStateRdSendStart:
    if (ETwiStateWrSendStart == TheTwiState) {
      TheTwiState = ETwiStateWrSendSlvAddr;
      /* send address (write) request */
      TWDR = (TheRequestAddress | TW_WRITE);
    } else /* if (ETwiStateRdSendStart == TheTwiState) */ {
      TheTwiState = ETwiStateRdSendSlvAddr;
      /* send address (read) request */
      TWDR = (TheRequestAddress | TW_READ);
    }
    /* TWI Interface enabled, Enable TWI Interupt and clear the flag to send byte */
    TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    break;
  default:
    break;
  }
}

static inline void hw_twi_handle_slave_address_transmitted(void)
{
  switch (TheTwiState) {
  case ETwiStateRdSendSlvAddr:
    TheTwiIndex = 0;
    if (1 != TheRequestLength) {
      /* more than 1 byte to receive, send ACK */
      TheTwiState = ETwiStateRdReadingBytes;
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    } else {
      TheTwiState = ETwiStateRdReadingBytesLast;
      /* only 1 byte to receive, send NACK */
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    }
    break;
  case ETwiStateWrSendSlvAddr:
    TheTwiIndex = 0;
    TheTwiState = ETwiStateWrWritingData;
    TWDR = *TheWriteBuffer;
    TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    break;
  default:
    /** @todo implement error handling */
    break;
  }
}

static inline void hw_twi_handle_data_transmitted(void)
{
  switch (TheTwiState) {
  case ETwiStateWrWritingData:
    if (TheRequestLength != (TheTwiIndex + 1)) {
      /* not all the data transmitted, send the next byte */
      ++TheTwiIndex;
      TWDR = TheWriteBuffer[TheTwiIndex];
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    } else {
      /* all the requested data sent, send the stop condition */
      TheTwiState = ETwiStateWrDone;
      TWCR = (1<<TWEN)|(0<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC);
    }
    break;
  default:
    /** @todo implement error handling */
    break;
  }
}

static inline void hw_twi_handle_data_received_ack(void)
{
  uint8_t data;
  switch (TheTwiState) {
  case ETwiStateRdReadingBytes:
    data = TWDR;
    TheReadBuffer[TheTwiIndex] = data;
    if ((TheRequestLength - 1) == ++TheTwiIndex) {
      /* request reading the last byte */
      /* TWI Interface enabled */
      TheTwiState = ETwiStateRdReadingBytesLast;
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    } else {
      /* request reading another byte */
      /* TWI Interface enabled */
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    }
    break;
  default:
    /** @todo implement error handling */
    break;
  }
}

static inline void hw_twi_handle_data_received_nack(void)
{
  uint8_t data;

  switch (TheTwiState) {
  case ETwiStateRdReadingBytesLast:
    data = TWDR;
    TheReadBuffer[TheTwiIndex] = data;
    /* TWI Interface enabled;
       Disable TWI Interrupt and clear the flag;
       Initiate a STOP condition. */
    TheTwiState = ETwiStateRdDone;
    TWCR = (1<<TWEN)|(0<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC);
    break;
  default:
    /** @todo implement error handling */
    break;
  }
}

ISR(TWI_vect)
{
  const uint8_t data = TW_STATUS;
  switch (data) {
  case TW_START:
  case TW_REP_START:
    hw_twi_handle_start_transmitted();
    break;
  case TW_MT_SLA_ACK:
    hw_twi_handle_slave_address_transmitted();
    break;
  case TW_MR_SLA_ACK:
    hw_twi_handle_slave_address_transmitted();
    break;
  case TW_MT_DATA_ACK:
    hw_twi_handle_data_transmitted();
    break;
  case TW_MR_DATA_ACK:
  case TW_SR_DATA_ACK:
    hw_twi_handle_data_received_ack();
    break;
  case TW_SR_DATA_NACK:
  case TW_MR_DATA_NACK:
    hw_twi_handle_data_received_nack();
    break;
  }
}
