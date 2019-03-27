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
/**
 * @file main.c
 * @brief test libznt_sock.so
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2019-03-19 Z.Riemann found
 */

#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/app/trace2console.h>
#include <znt/sock/sock.h>

void dump_errnos(int enable){
    if(!enable){
        return;
    }
    zerrno(ECONNABORTED);
    zerrno(EINTR);
    zerrno(EAGAIN);
    zerrno(EWOULDBLOCK);
    zerrno(EBADF);
    zerrno(EFAULT);
    zerrno(EINVAL);
    zerrno(EMFILE);
    zerrno(ENFILE);
    zerrno(ENOBUFS);
    zerrno(ENOMEM);
    zerrno(ENOTSOCK);
    zerrno(EOPNOTSUPP);
    zerrno(EPROTO);
    zerrno(EPERM);
}

void base_api(int enable){
    if(!enable){
        return;
    }
    zfd_t fd = zsocket(AF_INET, SOCK_STREAM, 0);
    zbind(fd, NULL);
    zlisten(fd, 5);
    zaccept(fd, NULL, NULL);
    zsock_close(fd);
}
int main(int argc, char **argv){
    ztrace_register(ztrace2console, NULL);
    zinf("Beging testing libznt_sock.so...");
    dump_errnos(1);
    base_api(1);
    zinf("testing done.");
    return 0;
}