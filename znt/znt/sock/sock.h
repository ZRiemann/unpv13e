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

#define ZSOCK_INVALID -1
typedef int zfd_t;
typedef unsigned short zport_t;
typedef struct sockaddr_storage zss_t;
typedef struct sockaddr zsa_t;
typedef socklen_t zsa_len_t;

ZC_BEGIN
ZAPI zfd_t zsocket(int family, int type, int protocol);
ZAPI void zsock_close(zfd_t fd);
zinline zsock_setnonblocking(zfd_t){
    int val = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
}

ZAPI zerr_t zbind(zfd_t fd, zsa_t *addr, socklen_t len);
ZAPI zerr_t zlisten(zfd_t fd, int queue_size);
ZAPI zerr_t zaccept(zfd_t fd, zsa_t* src, socklen_t *len, int *flag);

ZAPI zerr_t zreadn(zfd_t fd, void *buf, size_t *n);
ZAPI zerr_t zwritten(zfd_t fd, const void *buf, size_t *n);
ZAPI zerr_t zreadline(zfd_t fd, void *buf, size_t *n);
/* IPv4 + IPv6
 */
/* bind + listen */
ZAPI zerr_t zsock_passive(zfd_t fd, const char *addr, zport_t port);
/* numeric to presentation */
ZAPI zerr_t zsock_ntop(char* str, size_t len,
                       const zsa_t *sockaddr, socklen_t addrlen);
/* presentation to numeric */
ZAPI zerr_t zsock_pton(SA *sockaddr, socklen_t addrlen,
                       const char *str);
ZAPI zerr_t zsock_bind_wild(zfd_t fd, int family, zport_t *port);

ZC_END
#endif /*_ZNT_SOCK_H_*/
