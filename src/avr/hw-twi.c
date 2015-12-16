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

#include "hw-twi.h"

#include "hw-uart.h"
#include "scheduler.h"

#include <util/twi.h>
#include <avr/interrupt.h>

typedef enum {
    /* system states */
    EI2CStateNull = 0,
    EI2CStateIdle,
    EI2CStateCancelling,
    /* TWI reading states */
    EI2CStateRdSendStart,
    EI2CStateRdSendSlvAddr,
    EI2CStateRdReadingBytes,
    EI2CStateRdReadingBytesLast,
    EI2CStateRdDone,
    EI2CStateRdDoneError,
    /* TWI writing states */
    EI2CStateWrSendStart,
    EI2CStateWrSendSlvAddr,
    EI2CStateWrWritingData,
    EI2CStateWrDone,
    EI2CStateWrDoneError
} TwiCState;

typedef struct {
    uint8_t mAddress;
    uint8_t mRequestLenght;
    union {
        struct {
            uint8_t *mBuffer;
        } mRead;
        struct {
            const uint8_t *mBuffer;
        } mWrite;
    } mData;
} TClientRequest;

#define READ_BUFFER_LENGTH (20)

/**
 * The data represinting the current request
 */
static TClientRequest TheRequest;

static mcode_result TheCallback;
static uint8_t volatile TheI2CIndex = 0;
static uint8_t volatile TheI2CState = EI2CStateNull;
static uint8_t volatile TheReadBuffer[READ_BUFFER_LENGTH];

/**
 * The scheduler tick
 * @return TRUE if more work is already available
 */
static void hw_i2c_sched_tick(void);
/**
 * Send 'start' I2C protocol condition
 */
static inline void hw_i2c_send_start(void);

void i2c_init(void)
{
  TheI2CState = EI2CStateIdle;
  mcode_scheduler_add(hw_i2c_sched_tick);

  TWBR = 0x0CU;
  TWDR = 0xFFU;
  TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
  PORTC |= (1<<PC0) | (1<<PC1);
}

void i2c_deinit(void)
{
}

const uint8_t *i2c_get_read_buffer(void)
{
  return (const uint8_t *)TheReadBuffer;
}

void i2c_set_callback(mcode_result callback)
{
  TheCallback = callback;
}

void i2c_recv(uint8_t addr, uint8_t length)
{
  if (EI2CStateIdle == TheI2CState) {
    TheRequest.mAddress = (addr & 0xFEU);
    TheRequest.mRequestLenght = length;

    TheI2CState = EI2CStateRdSendStart;
    hw_i2c_send_start();
  }
  else {
    if (TheCallback) {
      hw_uart_write_string_P(PSTR("Error: wrong state: 0x"));
      hw_uart_write_uint(TheI2CState);
      hw_uart_write_string_P(PSTR("\r\n"));
      (*TheCallback)(false);
    }
  }
}

void i2c_send(uint8_t addr, uint8_t length, const uint8_t *data)
{
  if (EI2CStateIdle == TheI2CState) {
    TheRequest.mAddress = (addr & 0xFEU);
    TheRequest.mRequestLenght = length;
    TheRequest.mData.mWrite.mBuffer = data;

    TheI2CState = EI2CStateWrSendStart;
    hw_i2c_send_start();
  } else {
    (*TheCallback)(false);
  }
}

static void hw_i2c_sched_tick(void)
{
  switch (TheI2CState)
  {
  case EI2CStateWrDone:
    TheI2CState = EI2CStateIdle;
    hw_uart_write_string_P(PSTR("Write done.\r\n"));
    (*TheCallback)(true);
    break;
  case EI2CStateWrDoneError:
    hw_uart_write_string_P(PSTR("Write error.\r\n"));
    TheI2CState = EI2CStateIdle;
    (*TheCallback)(false);
    break;
  case EI2CStateRdDone:
    hw_uart_write_string_P(PSTR("Read done.\r\n"));
    TheI2CState = EI2CStateIdle;
    (*TheCallback)(true);
    break;
  case EI2CStateRdDoneError:
    hw_uart_write_string_P(PSTR("Read error.\r\n"));
    (*TheCallback)(false);
    break;
  default:
    break;
  }
}

static inline void hw_i2c_send_start(void)
{
    /* send the START condition */
    TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN)) | (1<<TWIE);
}

/**
 * Handle the event that the START condition is sent
 */
static inline void hw_i2c_handle_start_transmitted(void)
{
  switch (TheI2CState) {
  case EI2CStateWrSendStart:
  case EI2CStateRdSendStart:
    if (EI2CStateWrSendStart == TheI2CState) {
      TheI2CState = EI2CStateWrSendSlvAddr;
      /* send address (write) request */
      TWDR = (TheRequest.mAddress | TW_WRITE);
    } else /* if (EI2CStateRdSendStart == TheI2CState) */ {
      TheI2CState = EI2CStateRdSendSlvAddr;
      /* send address (read) request */
      TWDR = (TheRequest.mAddress | TW_READ);
    }
    /* TWI Interface enabled, Enable TWI Interupt and clear the flag to send byte */
    TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    break;
  default:
    break;
  }
}

static inline void hw_i2c_handle_slave_address_transmitted(void)
{
  switch (TheI2CState) {
  case EI2CStateRdSendSlvAddr:
    TheI2CIndex = 0;
    if (1 != TheRequest.mRequestLenght) {
      /* more than 1 byte to receive, send ACK */
      TheI2CState = EI2CStateRdReadingBytes;
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    } else {
      TheI2CState = EI2CStateRdReadingBytesLast;
      /* only 1 byte to receive, send NACK */
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    }
    break;
  case EI2CStateWrSendSlvAddr:
    TheI2CIndex = 0;
    TheI2CState = EI2CStateWrWritingData;
    TWDR = TheRequest.mData.mWrite.mBuffer[0];
    TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    break;
  default:
    /** @todo implement error handling */
    break;
  }
}

static inline void hw_i2c_handle_data_transmitted(void)
{
  switch (TheI2CState) {
  case EI2CStateWrWritingData:
    if (TheRequest.mRequestLenght != (TheI2CIndex + 1)) {
      /* not all the data transmitted, send the next byte */
      ++TheI2CIndex;
      TWDR = TheRequest.mData.mWrite.mBuffer[TheI2CIndex];
      TWCR = (1<<TWEN)|(1<<TWIE)|(1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC);
    } else {
      /* all the requested data sent, send the stop condition */
      TheI2CState = EI2CStateWrDone;
      TWCR = (1<<TWEN)|(0<<TWIE)|(1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC);
    }
    break;
  default:
    /** @todo implement error handling */
    break;
  }
}

static inline void hw_i2c_handle_data_received_ack(void)
{
  uint8_t data;
  switch (TheI2CState) {
  case EI2CStateRdReadingBytes:
    data = TWDR;
    TheReadBuffer[TheI2CIndex] = data;
    if ((TheRequest.mRequestLenght - 1) == ++TheI2CIndex) {
      /* request reading the last byte */
      /* TWI Interface enabled */
      TheI2CState = EI2CStateRdReadingBytesLast;
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

static inline void hw_i2c_handle_data_received_nack(void)
{
  uint8_t data;

  switch (TheI2CState) {
  case EI2CStateRdReadingBytesLast:
    data = TWDR;
    TheReadBuffer[TheI2CIndex] = data;
    /* TWI Interface enabled;
       Disable TWI Interrupt and clear the flag;
       Initiate a STOP condition. */
    TheI2CState = EI2CStateRdDone;
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
    hw_i2c_handle_start_transmitted();
    break;
  case TW_MT_SLA_ACK:
    hw_i2c_handle_slave_address_transmitted();
    break;
  case TW_MR_SLA_ACK:
    hw_i2c_handle_slave_address_transmitted();
    break;
  case TW_MT_DATA_ACK:
    hw_i2c_handle_data_transmitted();
    break;
  case TW_MR_DATA_ACK:
  case TW_SR_DATA_ACK:
    hw_i2c_handle_data_received_ack();
    break;
  case TW_SR_DATA_NACK:
  case TW_MR_DATA_NACK:
    hw_i2c_handle_data_received_nack();
    break;
  }
}
