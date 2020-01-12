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

#include "hw-nvm.h"

#include "stm32f10x_bkp.h"

static uint16_t index_to_address(uint16_t index);

uint16_t nvm_read(uint_least8_t index)
{
  return BKP_ReadBackupRegister(index_to_address(index));
}

void nvm_write(uint_least8_t index, uint16_t value)
{
  BKP_WriteBackupRegister(index_to_address(index), value);
}

uint16_t index_to_address(uint16_t index)
{
  const uint16_t idx_to_addr_map[] = {
    BKP_DR1, BKP_DR2, BKP_DR3, BKP_DR4,
    BKP_DR5, BKP_DR6, BKP_DR7, BKP_DR8,
    BKP_DR9, BKP_DR10,
  };

  if (index < 10) {
    return idx_to_addr_map[index];
  } else {
    return -1;
  }
}
