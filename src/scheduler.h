/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2018 Alexander Chumakov
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

#ifndef MCODE_SCHEDULER_H
#define MCODE_SCHEDULER_H

#include "mglobal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the scheduler
 */
void scheduler_init(void);

/**
 * Deinitialize the scheduler
 */
void scheduler_deinit(void);

/**
 * Start the scheduler in the current call stack position
 */
void scheduler_start(void);

/**
 * Stop the scheduler in the last call stack position
 */
void scheduler_stop(void);

/**
 * Add a new event handler
 */
void scheduler_add(mcode_tick tick);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_SCHEDULER_H */
