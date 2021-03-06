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

#ifndef MCODE_HW_TWI_H
#define MCODE_HW_TWI_H

#include "mcode-config.h"

#include "mglobal.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MCODE_TWI

void twi_init(void);
void twi_deinit(void);

void twi_recv(uint8_t addr, uint8_t length, mcode_read_ready callback);
void twi_send(uint8_t addr, uint8_t length, const uint8_t *data, mcode_done callback);

bool twi_recv_sync(uint8_t addr, uint8_t length, uint8_t *data);
bool twi_send_sync(uint8_t addr, uint8_t length, const uint8_t *data);

#endif /* MCODE_TWI */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_HW_TWI_H */
