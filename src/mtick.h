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

#ifndef MCODE_MTICK_H
#define MCODE_MTICK_H

#include "mglobal.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Initialize the milli-second scheduler
 */
void mtick_init(void);
/*!
 * Deinitialize the milli-second scheduler
 */
void mtick_deinit(void);
/*!
 * Add a new loop to the scheduler
 * @param[in] tick The callback to be called each milli-second
 *
 */
void mtick_add(mcode_tick tick);

/*!
 * Suspend the main thread for mticks milli-seconds
 * @param[in] mticks The number of milli-seconds to sleep
 */
void mtick_sleep(uint32_t mticks);

/*!
 * Get the uptime in milliseconds
 * @return The number of milli-seconds since the last power-on
 */
uint64_t mtick_count(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_MTICK_H */
