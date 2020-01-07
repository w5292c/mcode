/* librock_sha256.c, an implementation of SHA-256 in C.

Part of libROCK: QUICK REUSE WITHOUT CHANGES!
- MIT License
- High-Quality. Highly portable. Compiles on gcc/MSVC/Clang/Windows/Linux/BSD/more.
- Global names start "librock_", for compatibility.

This file consists of
    [[The MIT License]] (and copyright statement.)

    [[okdshin's C++ picosha2, adapted to C. static (PRIVATE)]]

    [[librock_sha256, (PUBLIC)]]
        typedef struct librock_SHA256_CTX *librock_SHA256_CTX_t; // Opaque structure.

        int librock_SHA256_Init(
            struct librock_SHA256_CTX *c); // Call with NULL to get a size to allocate

        int librock_SHA256_Update(
            struct librock_SHA256_CTX *c, const void *data, int len);

        int librock_SHA256_StoreFinal (
            unsigned char *md, struct librock_SHA256_CTX *c); //md=32 bytes

    [[Typical example main()]] #ifdef LIBROCK_SHA256_MAIN
*/

/**************************************************************/
//[[The MIT License]]
/*
The MIT License (MIT)

Portions Copyright (C) 2014 okdshin
Portions Copyright (C) 2016 MIB SOFTWARE INC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
/**************************************************************/

/**************************************************************/
//[[okdshin's picosha2, adapted to C. Static (PRIVATE)]]
/* Adapted from picosha2.h, a C++, header-only implementation at:
    https://github.com/okdshin/PicoSHA2/blob/master/picosha2.h

    Adapted to C by Forrest Cavalier III, MIB SOFTWARE INC.
*/

/**************************************************************/
//[[librock_sha256]] By Forrest Cavalier III, MIB SOFTWARE, INC.
/* See MIT LICENSE above for copyright and license statement.*/

/*
 * The MIT License (MIT)
 * (c) 2020 Alexnader Chumakov
 * Copied from 'librock_sha256.c', adapted to contain only API definitions
 */

#ifndef MCODE_SECURITY_SHA256_H_
#define MCODE_SECURITY_SHA256_H_

#include <stdint.h>

typedef uint32_t word_t;
typedef uint8_t byte_t;

#define MD_LENGTH_SHA256 (32)

typedef struct librock_SHA256_CTX {
    word_t data_length_digits_[4]; //as 64bit integer (16bit x 4 integer)
    word_t h_[8];
    uint8_t buffer[64];
    int nBuffer;
} SHA256_CTX;

int librock_SHA256_Init(SHA256_CTX *c);

int librock_SHA256_Update(SHA256_CTX *c, const void *data, int len);

int librock_SHA256_StoreFinal (unsigned char *md, SHA256_CTX *c);

void sha256(const void *data, int length, uint8_t *md);

#endif /* MCODE_SECURITY_SHA256_H_ */
