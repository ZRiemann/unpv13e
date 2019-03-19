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
#ifndef _ZNT_SOCK_H_
#define _ZNT_SOCK_H_

/**
 * @file sock.h
 * @brief socket API wrapper
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2019-03-19 Z.Riemann found
 */
#include <zsi/base/type.h>
#include "unp.h"

typedef int zfd_t;
ZC_BEGIN
ZAPI zfd_t zsocket(int family, int type, int protocol);
ZAPI void zsock_close(zfd_t fd);

ZAPI zerr_t zbind(zfd_t fd, const char *addr);
ZAPI zerr_t zlisten(zfd_t fd, int queue_size);
ZAPI zerr_t zaccept(zfd_t fd, SA* src, socklen_t *len);
ZC_END
#endif /*_ZNT_SOCK_H_*/
