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
 * @file transport.c
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2019-04-02 Z.Riemann found
 */
#include <sys/epoll.h>
#include <zsi/base/time.h>
#include <zsi/base/error.h>

#define ZTRACE_TITLE "[TRANSPORT]"
#define ZTRACE_LEVEL_CTL 0x1f
#include <zsi/base/trace.h>

#define ENABLE_REFER 1
#define ENABLE_DATA_POS 1
#define ENABLE_LIST 1
#include <znt/sock/transport.h>

static zbuf_t *g_buf;
#define MAX_EVENTS 10

static zerr_t ztns_on_accept(ztns_t *tns, ztns_conn_t *event){
    zerr_t ret = ZEOK;
    ztns_conn_t conn = {0};
    zss_t ss = {0};
    zsa_len_t len= 0;
    int flag = 0;
    char peer_ip[128];
    zport_t peer_port = 0;
    struct epoll_event ev = {0};
    for(;;){
        flag = SOCK_NONBLOCK | SOCK_CLOEXEC;
        // accept all queued connection
        if(ZSOCK_INVALID == (conn.fd = zaccept(event->fd, (zsa_t*) &ss, &len, &flag))){
            if(flag == EAGAIN){
                zlog(ZDBG, ZE_EOF);
                break;
            }else{
                zerrno(ZEFAIL);
                break;
            }
        }else if(event->user){
            conn->in = (zbuf_head_t*)&ss;
            conn->in_cur = (zbuf_head_t*)&len;

            if(ZEOK == (ret = zsock_ntop(peer_ip, 128, (zsa_t*)&ss, len))){
                zmsg("accept connection: %s:%d", peer_ip, peer_port);
            }else{
                zerrno(ret);
            }
            ret = ((ztns_on_event*)event->user)(&conn);
            // reset connection
            conn->in = NULL;
            conn->in_cur = NULL;
        }
        if(ZEOK == ret){
            // add connection to poll wait
            ztns_conn_t *pconn = zbuf_alloc_ptr(g_buf, sizeof(ztns_conn_t));
            if(!pconn){
                zsock_close(conn.fd);
                ret = ZEMEM_INSUFFICIENT;
                zerrno(ret);
                break;
            }
            zmsg("TODO: set socket send/receive buffer size with 16KB....");
            memcopy(pconn, &conn, sizeof(ztns_conn_t));
            ev.events = EPOLLIN | EPOLLET;
            ev.data.ptr = (zptr_t)pconn;
            if( -1 == epoll_ctl(tns->epoll_fd, EPOLL_CTL_ADD, conn.fd, &ev)){
                ret = errno;
                zerrno(ret);
            }else{
                zmsg("add to epoll control: %s:%d", peer_ip, peer_port);
            }
        }else{
            // user reject the connection
            zwar("reject the connection: %s:%d", peer_ip, peer_port);
            zsock_close(conn.fd);
        }
    }
    return ret;
}

static zthr_ret_t ZCALL ztns_poll_proc(void* param){
    zerr_t ret;
    int ready;
    ztns_t *tns;
    zfd_t epoll_fd;
    ztns_conn_t *event;
    struct epoll_event events[MAX_EVENTS];

    tns = (ztns_t*)param;
    tns->stop = 2;
    if(-1 == (epoll_fd = epoll_create1(EPOLL_CLOEXEC))){
        zerrno(errno);
        return (zthr_ret_t)-1;
    }else{
        tns->epoll_fd = epoll_fd;
        zmsg("create epoll_fd<%d>", epoll_fd);
    }

    for(;;){
        ready = epoll_pwait(epoll_fd, events, MAX_EVENTS, 3000, NULL);
        if(ready > 0){
            // a file descriptor delivers an event;
            while(ready >= 0){
                event = (ztns_conn_t*)events[ready].data.ptr;
                if(event->events & EPOLLOUT){
                    // handle POLLOUT
                    // if send buffer done, clear EPOLLOUT flag
                }
                if(event->events & EPOLLIN){
                    // handle POLLIN
                    ret = event->on_event(event);
                    // if(ret = ZE_TNS_CLOSE){ztns_close(event);}
                    // close event may be occur asynchronously post by UNIX socket.
                }
                --ready;
            }
        }else if(ready < 0){
            // an error occurs
            zerrno(errno);
            if(errno != EINTR){
                break;
            }
        }else{
            // timeout
            zlog(ZDBG, ZETIMEOUT);
            if(1 == tns->stop){
                break;
            }
        }
    }
    close(epoll_fd);
    // reset tns
    tns->epoll_fd = ZSOCK_INVALID;
    tns->stop = 0;
    zlog(ZMSG, ZE_EOF);
    return (zthr_ret_t)0;
}
/**
 * @brief create transport handle
 * @param [out] tns transport handle
 * @retval ZEOK
 * @retval ZEMEM_INSUFFICIENT
 */
zerr_t ztns_create(ztns_t **tns){
    zbuf_head_t *head;
    // assert tns...
    if(NULL == (g_buf = zbuf_create())){
        zerrno(ZEMEM_INSUFFICIENT);
        return ZEMEM_INSUFFICIENT;
    }
    if(NULL == (*tns = (ztns_t*)zbuf_alloc_ptr(g_buf, sizeof(ztns_t)))){
        zerrno(ZEMEM_INSUFFICIENT);
        return ZEMEM_INSUFFICIENT;
    }
    (*tns)->epoll_fd = ZSOCK_INVALID;
    zerrno(ZEOK);
    return ZEOK;
}
/**
 * @brief destroy transport handle
 * @param [in] tns The transport handle which created by ztns_create()
 * @retval ZEOK
 * @retval ZEPARAM_INVALID
 */
zerr_t ztns_destroy(ztns_t *tns){
    // assert tns,g_buf...
    // free buffers...
    zwar("TODO: deep free buffers in <tns>");
    zbuf_head_t *head = (zbuf_head_t*)zmember2st(zbuf_head_t, payload, tns);
    zbuf_free(g_buf, head);
    zerrno(ZEOK);
    return ZEOK;
}
/**
 * @brief startup transport server
 * @param [in] tns Transport handle
 */
zerr_t ztns_init(ztns_t *tns){
    zerr_t ret = ZEOK;
    // create poll thread
    if(tns->epoll_fd != ZSOCK_INVALID || tns->stop != 0){
        zerrno(ZESTATUS_INVALID);
        return ZESTATUS_INVALID;
    }
    ret = zthread_create(&tns->thr, ztns_poll_proc, (void*)tns);
    zerrno(ret);
    return ret;
}
/**
 * @brief stop transport server
 * @param [in] tns Transport handle
 */
zerr_t ztns_fini(ztns_t *tns){
    zerr_t ret = ZEOK;
    // stop poll thread
    tns->stop = 1;
    ret = zthread_join(&tns->thr);
    zerrno(ret);
    return ret;
}
/**
 * @brief Connect to <addr>
 * @param [in] tns The transport handle
 * @param [in] addr destination socket address
 * @param [in] onconn connection callback on nonblocking socket
 * @retval ZEOK
 * @retval ZEPARAM_INVALID
 */
zerr_t ztns_connect(ztns_t *tns, zsa_t *addr, ztns_on_event onconn){
    zerrno(ZENOT_SUPPORT);
    return ZENOT_SUPPORT;
}
/**
 * @brief startup passive connection
 * @param [in] tns The transport handle
 * @param [in] addr peer connection address
 * @param [in] onaccept new connection callback
 * @retval ZEOK
 * @retval ZEMEM_INSUFFICIENT
 * @retval ZENOT_INIT
 */
zerr_t ztns_listen(ztns_t *tns, zsa_t *addr, zsa_len_t len, ztns_on_accept on_accept){
    zerr_t ret = ZEOK;
    ztns_conn_t *conn = (ztns_conn_t*)zbuf_alloc_ptr(g_buf, sizeof(ztns_conn_t));

    do{
        if(!conn){
            ret = ZEMEM_INSUFFICIENT;
            break;
        }
        if(ZSOCK_INVALID == tns->epoll_fd){
            zmsg("wait poll thread to create epoll_fd");
            zsleepms(1);
            if(ZSOCK_INVALID == tns->epoll_fd){
                ret = ZENOT_INIT;
                break;
            }
        }
        if(AF_INET == addr->sa_family){
            struct epoll_event ev = {0};
            conn->fd = zsocket(AF_INET, SOCK_STREAM, 0);
            if(ZSOCK_INVALID == conn->fd){
                ret = ZEFAIL;
                break;
            }
            zsock_setnonblocking(conn->fd);
            if(ZEOK != (ret = zbind(conn->fd, addr, len))){
                break;
            }

            if(ZEOK != (ret = zlisten(conn->fd, 1024))){
                break;
            }

            ev.events = EPOLLIN;
            ev.data.ptr = (zptr_t)conn;
            if(-1 == (epoll_ctl(tns->epoll_fd, EPOLL_CTL_ADD, conn->fd, &ev))){
                ret = errno;
                zerrno(ret);
                break;
            }

            conn->on_event = ztns_on_accept;
            conn->user = (zptr_t)on_accept;
        }else{
            zerr("not support socket family: %d", addr->sa_family);
            ret = ZENOT_SUPPORT;
        }
    }while(0);

    if(ZEOK != ret){
        zbuf_free_ptr(g_buf, conn);
    }
    zerrno(ret);
    return ret;
}
/**
 * @brief send a packet
 * @param [in] conn connection handle
 * @param [in] pkt send buffer
 * @retval ZEOK
 */
zerr_t ztns_send(ztns_conn_t *conn, ztns_pkt_t *pkt){
    zerrno(ZENOT_SUPPORT);
    return ZENOT_SUPPORT;
}
/**
 * @brief recveive a packet(list)
 * @param [in] conn connection handle
 * @param [out] pkt receive buffer
 */
zerr_t ztns_recv(ztns_conn_t *conn, ztns_pkt_t *pkt){
    
    return ZENOT_SUPPORT;
}
/**
 * @brief Close the connection
 * @param conn Connection handle to be closed
 * @retval ZEOK
 */
zerr_t ztns_close(ztns_conn_t *conn){
    zerrno(ZENOT_SUPPORT);
    return ZENOT_SUPPORT;
}

/**
 * @brief alloc a packet(list)
 * @param head packet list header, if NULL it will alloc single packet
 * @return the new packet and append to list tail
 * @remark size = ZTNS_PKT_SIZE = 16384
 */
ztns_pkt_t *ztns_pkt_alloc(ztns_pkt_t *head){
    ztns_pkt_t *pkt = zbuf_stack_alloc(g_buf, 14, 16384);
    zbuf_push(head, pkt);
    return NULL;
}

ztns_pkt_t *ztns_pkt_fetch(ztns_conn_t *conn){
    ztns_pkt_t *pkt;
    if(conn->in){
        pkt = comm->in; // fetch stored data
        conn->in = NULL; // reset it, avoid memory leak
    }else{
        pkt = ztns_pkt_alloc(NULL); // no buffered data
    }
    return pkt;
}

void ztns_pkt_store(ztns_conn_t *conn, ztns_pkt_t *pkt){
    conn->in = pkt;
}

/**
 * @brief add refers to packet(list)
 */
void ztns_pkt_refer(ztns_pkt_t *pkt){
    zbuf_refer(pkt);
}
/**
 * @brief deep copy packet(list)
 */
ztns_pkt_t *ztns_pkt_clone(ztns_pkt_t *pkt){
    zerrno(ZENOT_SUPPORT);
    return NULL;
}
/**
 * @brief recycle the packet(list)
 */
void ztns_pkt_free(ztns_pkt_t *pkt){
    zbuf_free_list(g_buf, pkt);
}
/**
 * @brief write <data> to <pkt> list
 */
ztns_pkt_t *ztns_pkt_write(ztns_pkt_t *pkt, zptr_t data, size_t len){
    zerrno(ZENOT_SUPPORT);
    return NULL;
}
/**
 * @brief get the current packet<pkt> data pointer and data length
 * @param [in] pkt The current packet
 * @param [out] data Buffer data pointer
 * @param [out] len Buffer data length
 * @ratval NULL No more packets
 * @retval !NULL The next packet list item
 */
ztns_pkt_t *ztns_pkt_data(ztns_pkt_t *pkt, zptr_t *data, size_t *len){
    zerrno(ZENOT_SUPPORT);
    return NULL;
}
/**
 * @brief splice packet list<pkt>
 * @param [in] pkt The packet list
 * @param [out] spliced_pkt The spliced packet
 * @retval ZEOK
 * @retval ZEMEM_INSUFFICIENT
 */
zerr_t ztns_pkt_splice(ztns_pkt_t *pkt, ztns_pkt_t **spliced_pkt){
    zerrno(ZENOT_SUPPORT);
    return ZENOT_SUPPORT;
}
