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
 *      -# linux epoll(edge-trigger)/aio, windows iocp
 *      -# session timer control
 *      -# buffered
 *      -# bind_cpu create()->tns.bind_cup->init()
 *      -# EFFICIENCY FIRST, codes need be well designed.
 */
#include <znt/sock/sock.h>
#include "buffer.h"

ZC_BEGIN

#define ZTNS_PKT_SIZE ZBUF_MAX_VALUE
typedef zbuf_head_t ztns_pkt_t; /** transport packet type */
typedef struct timeval ztimestamp_t;
typedef void* zbuffer_t;

typedef struct ztransport_connection_s{
    zfd_t fd;
    ztimestamp_t conn; /** connection establish time */
    ztimestamp_t active; /** last active time */
    ztimestamp_t wait; /** receive timeout timestamp */
    zbuf_head_t *in; /** pollin buffer queue */
    zbuf_head_t *in_cur; /** current buffer block */
    zbuf_head_t *out; /** pollout buffer queue */
    zbuf_head_t *out_cur; /** current output block */
    zptr_t user; /** user data */
    /*
     * asynchronous events
     */
    zerr_t (*on_event)(struct ztransport_connection_t *conn);
    char extension[0]; /* extension */
}ztns_conn_t;

typedef struct ztransport_s{
    zconn_t *conns;
    /*
     * tuning transport layer
     */
    int bind_cpu; /* 0-default */
    char extension[0]; /* extension */
}ztns_t;

typedef zerr_t (*ztns_on_event)(ztns_conn_t *conn);
typedef zerr_t (*ztns_on_accept)(ztns_conn_t *conn, zsa_t *addr, zsa_len_t len);
/*
 * net APIs
 */
ZAPI zerr_t ztns_create(ztns_t **tns);
ZAPI zerr_t ztns_destroy(ztns_t *tns);
ZAPI zerr_t ztns_init(ztns_t *tns);
ZAPI zerr_t ztns_fini(ztns_t *tns);

ZAPI zerr_t ztns_connect(ztns_t *tns, zsa_t *addr, ztns_on_event onconn);
ZAPI zerr_t ztns_listen(ztns_t *tns, zsa_t *addr, ztns_on_accept onaccept);
ZAPI zerr_t ztns_send(ztns_conn_t *conn, ztns_packet_t *pkt);
ZAPI zerr_t ztns_recv(ztns_conn_t *conn, ztns_packet_t *pkt);
ZAPI zerr_t ztns_close(ztns_conn_t *conn);
/*
 * packet manager APIs
 */
ZAPI ztns_pkt_t *ztns_pkt_alloc(ztns_pkt_t *head); /* size = ZTNS_PKT_SIZE = 16384 */
ZAPI void ztns_pkt_refer(ztns_pkt_t *pkt); /* add packet refers*/
ZAPI znts_pkt_t *ztns_pkt_clone(ztns_pkt_t *pkt) /* deep copy packet */
ZAPI void ztns_pkt_free(ztns_pkt_t *pkt);
ZAPI ztns_pkt_t *ztns_pkt_write(ztns_pkt_t *pkt, zptr_t data, size_t len);
ZAPI ztns_pkt_t *ztns_pkt_data(ztns_pkt_t *pkt, zptr_t *buf, size_t *len);
/**
 * @brief splice packet list to packet
 * @remark It will cause memory copy and memory allocate, try avoid to use it.
 */
ZAPI zerr_t ztns_pkt_splice(ztns_pkt_t *pkt, ztns_pkt_t **pkt);
ZC_END

#endif /*_Z_TRANSPORT_H_*/

#if 0 // sample
int is_server = 1;
zerr_t on_event_imp(ztns_conn_t *conn){
    // handle pollout event in backgournd
    void *buf;
    printf("pollin events");
    // post pollin task to other thread
    // or single thread;
    ztns_recv(ztns_conn, &buf);
    // handle readed messages...
    // or handle close/error events background
}

zerr_t on_accept(){
    zaccept();
};
zerr_t on_conn_imp(ztns_conn_t *conn){
    if(is_server){
        return on_accept();
    }else{
        printf("connection established.");
        conn->on_event = on_event_imp();
    }
    return ZEOK;
}

zerr_t on_accept_imp()
int main(){
    ztns_t *tns;
    ztns_create(&tns);
    // tuning tns...
    tns->bind_cpu=1;
    tns->on_accept() = on_accept_imp;
    // ...
    ztns_init(tns);
    if(is_server){
        ztns_listen(tns, addr, on_conn_imp); // on_conn() return new connection
    }else{
        ztns_connect(tns, addr1, on_conn_imp); // on_conn() return the connection result
        for(;;){
            // post messages
            for(int i=0; i< tns->conns; i++){
                ztns_send(tns->conns[i], "hello");
            }
        }
    }
    ztns_fini();
}
#endif
