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

#ifndef MCODE_TIMER_H
#define MCODE_TIMER_H

#include "mglobal.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCODE_TIMER_HANDLERS
/** Default number of timer handlers */
#define MCODE_TIMER_HANDLERS (8)
#endif /* MCODE_TIMER_HANDLERS */

/*!
 * Initialize the milli-second scheduler
 */
void mtimer_init(void);
/*!
 * Deinitialize the milli-second scheduler
 */
void mtimer_deinit(void);
/*!
 * Add a new task to the timer to be invoked in the future
 * @param[in] handler The timer callback to be invoked after \msec milliseconds
 * @param[in] start The number of milliseconds to wait before the \c handler is called
 * @note The return value of the task is ingored for a non-periodic handle/task
 */
void mtimer_add(mcode_exec task, uint32_t start);

/**
 * Add a new periodic timer task to start in \c start msecs and \c period
 * @param[in] task The timer handler
 * @param[in] start The start time in milliseconds
 * @param[in] period The period for the task in milliseconds
 * @note If the periodic task handler returns \c false, it will be cancelled/removed,
 *       if it returns \c true, the handler will be called again after \c period milliseconds.
 */
void mtimer_add_periodic(mcode_exec task, uint32_t start, uint32_t period);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_TIMER_H */
