/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Alexander Chumakov
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

#ifndef MCODE_GSM_ENGINE_H
#define MCODE_GSM_ENGINE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  MGsmEventNone,
  MGsmEventSmsSent,
  MGsmEventSmsArrived,
} MGsmEvent;

/**
 * Callback for receiving events from the GSM engine
 * @param[in] The event type
 * @param[in] from The optional 'from' address for the event
 * @param[in] body The optional event body
 */
typedef void (*gsm_callback)(MGsmEvent type, const char *from, const char *body);

/**
 * Initialize the GSM engine
 */
void gsm_init(void);
/**
 * Initialize harware-specific GSM engine part
 */
void hw_gsm_init(void);
/**
 * Deinitialize the GSM engine
 */
void gsm_deinit(void);
/**
 * Deinitialize harware-specific GSM engine part
 */
void hw_gsm_deinit(void);

/**
 * Power on/off the GSM module
 */
void gsm_power(bool on);
/**
 * Hardware-specific power on/off the GSM module
 */
void hw_gsm_power(bool on);

/**
 * Set the GSM callback
 * @param[in] callback The callback for receiving GSM events
 */
void gsm_set_callback(gsm_callback callback);

/**
 * Power on/off the GSM module
 */
void gsm_power(bool on);

/**
 * Send AT-command to the GSM module
 * @param[in] cmd The AT command to be sent
 * @return Success of operation
 */
bool gsm_send_cmd(const char *cmd);

/**
 * Send an AT command to GSM module
 * @param[in] cmd The AT command to be sent, it may include escape sequences
 * @note Supported escape sequences:
 *       "\\r", "\\n", "\\t", "\\0",
 *       "\\a", "\\b", "\\v", "\\f",
 *       "\\e"
 * @note The call automatically adds the end-of-line marker ("\r\n"),
 *       no need to include it explicitly
 * @note This call does not check GSM Engine state, can break the GSM state machine
 */
void gsm_send_cmd_raw(const char *cmd);

/**
 * Send an SMS with 'body' to 'address'
 * @param[in] address The phone number for sending SMS
 * @param[in] body The SMS body to be sent
 * @return Success of operation
 */
bool gsm_send_sms(const char *address, const char *body);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_GSM_ENGINE_H */
