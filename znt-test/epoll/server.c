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
 * @file server.c
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2019-03-30 Z.Riemann found
 */
#include <stdio.h>
#include <sys/epoll.h>
#include <znt/sock/sock.h>

#define MAX_EVENTS 10

#define ZEPOLL_ASYNC 0 // post epoll events to other threads

/**
 * @brief Be used to save epoll event data
 */
typedef struct zepoll_event_s{
    zfd_t fd;
    // active timestamp
    // wait timestamp
    // send buf
    // recv buf
    // userdata...
}zepev_t;

/**
 * @brief Be used to manager zepev_ts
 * @par control session timeout
 */
typedef struct zepoll_events_s{
    int (*on_event1)(zfd_t);
    int on_event;
}zepevs_t;

int on_event_imp(zfd_t fd){
    printf("on_event_imp(%d)\n", fd);
}
void setnonblocking(zfd_t sockfd){
    int val = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, val | O_NONBLOCK);
}

/**
 * @param [in,out] zfd_t listening sockets
 */
//int on_epoll_init(zfd_t, zfd_t **, size_t *len){}
/**
 * @retval 0 ok
 * @retval 1 close
 * @retval 2 error
 * @retval 3 epoll_mod rw
 * @retval 4 epoll_mod r
 */
int on_event(zfd_t fd){
    // post to other thread(semaphore)
    // or
    static char buf[128];
    ssize_t n;
    int ret;
    for(;;){
        n = recv(fd, buf, 128, 0);
        if(n == 0){
            printf("client closed connection\n");
            ret = 1;
            break;
        }else if(n < 0){
            ret = errno == EAGAIN ? 0 : 2;
            break;
        }else{
            buf[n] = 0;
            printf("recv: %s\n", buf);
            send(fd, "epoll", 5, 0);
            printf("send: epoll\n");
            ret = 0;
        }
    }
    return ret;
}

void on_accept(zfd_t fd){
}

#if ZEPOLL_ASYNC
/**
 * @brief asynchronous control for socket close or error
 * @par thread safe?
 *     man epoll_wait(), select2()
 *     If a file descriptor being monitored by select() is closed in another
 *     thread, the result is unspecified.any application that relies on a
 *     particular behavior in this scenario must be considered buggy.
 */
void on_async_control(){
    
}
#endif // ZEPOLL_ASYNC

int main(int argc, char **argv){
    int n, nfds, ret;
    zsa_t addr;
    socklen_t addrlen;
    zfd_t listen_sock, conn_sock, epollfd;
    struct epoll_event ev, events[MAX_EVENTS];
    struct sockaddr_in	servaddr;

    zepevs_t epevs;
    epevs.on_event = 0;
    epevs.on_event1 = on_event_imp;
    epevs.on_event1(1);

    /* Code to set up listening socket, 'listen_sock',
       (socket(), bind(), listen()) omitted */
    listen_sock = zsocket(AF_INET, SOCK_STREAM, 0);
    setnonblocking(listen_sock);
    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(13000);	/* daytime server */

    zbind(listen_sock, (zsa_t*)&servaddr, sizeof(servaddr));
    listen(listen_sock, 1024);

#if ZEPOLL_ASYNC
    // create UNIX socket to transmit control message
#endif // ZEPOLL_ASYNC
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                // TODO: while(!EAGAIN){accept(...)}
                conn_sock = accept(listen_sock,
                                   (struct sockaddr *) &addr, &addrlen);
                if (conn_sock == -1) {
                    perror("accept");
                    // on_accept_error();
                    exit(EXIT_FAILURE);
                }
                on_accept(conn_sock);
                setnonblocking(conn_sock);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_sock;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock,
                              &ev) == -1) {
                    perror("epoll_ctl: conn_sock");
                    exit(EXIT_FAILURE);
                }else{
                    printf("epoll_ctl add %d\n", conn_sock);
                }
            }
#if ZEPOLL_ASYNC
            else if(0){
                printf("do nothing\n");
            }
#endif
            else {
#if ZEPOLL_ASYNC
                on_event(events[n].data.fd);
#else
                ret = on_event(events[n].data.fd);
                printf("ret = %d\n", ret);
                if(ret != 0){
                    printf("closed %d\n", events[n].data.fd);
                    close(events[n].data.fd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[n].data.fd, &ev);
                }
#endif
            }
        }
    }
}