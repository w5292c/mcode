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

#ifndef MCODE_PWM_H
#define MCODE_PWM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_ID_OC1A UINT8_C(0)
#define PWM_ID_OC1B UINT8_C(1)
#define PWM_ID_OC2  UINT8_C(2)

void pwm_init(void);

/**
 * Sets the PWM phase value.
 * @param[in] id The ID of the PWM. 0 corresponds to OC1A, 1 - OC1B, 2 - OC2
 * @param[in] value The PWM value to be set, may be from 0 to 255
 */
void pwm_set(uint8_t id, uint8_t value);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MCODE_PWM_H */
