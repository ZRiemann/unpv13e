/**
 * MIT License
 *
 * Copyright (c) 2018 Z.Riemann
 * https://github.com/ZRiemann/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the Software), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM
 * , OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _Z_TRANSPORT_H_
#define _Z_TRANSPORT_H_

/**
 * @file transport.h
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2019-03-30 Z.Riemann found
 * @par Synopsys
 *      1. linux epoll/aio, windows iocp
 *      2. session timer control
 *      3. epoll edge-triggered
 *      4. buffered
 */
#include <znt/sock/sock.h>

ZC_BEGIN
ZAPI ztns_connect(zsa_t *addrs);
ZC_END

#endif /*_Z_TRANSPORT_H_*/
