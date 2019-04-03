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
#include <zsi/mem/buffer.h>
#include <zsi/thread/thread.h>
#include <znt/sock/sock.h>

ZC_BEGIN

#define ZTNS_PKT_SIZE ZBUF_MAX_VALUE
typedef zbufs_t ztns_pkt_t; /** transport packet type */
typedef struct timeval ztimestamp_t;

typedef struct ztransport_connection_s{
    zfd_t fd;
    ztimestamp_t conn; /** connection establish time */
    ztimestamp_t active; /** last active time */
    ztimestamp_t wait; /** receive timeout timestamp */
    zbuf_head_t *in; /** pollin buffer queue */
    zbuf_head_t *in_cur; /** current buffer block */
    zbuf_head_t *out; /** pollout buffer queue */
    zbuf_head_t *out_cur; /** current output block */
    zptr_t user; /** user defined data */
    /*
     * asynchronous events
     */
    zerr_t (*on_event)(struct ztransport_connection_s *conn);
    char extension[0]; /* extension */
}ztns_conn_t;

typedef struct ztransport_s{
    zfd_t epoll_fd;
    ztns_conn_t *conns;
    /*
     * tuning transport layer
     */
    int bind_cpu; /* 0-default */
    /*
     * Internal members
     */
    zthr_t thr; /** background thread */
    int stop; /** stop flag */
    char extension[0]; /* extension */
}ztns_t;

typedef zerr_t (*ztns_on_event)(ztns_t *tns, ztns_conn_t *conn);
/*
 * net APIs
 */
ZAPI zerr_t ztns_create(ztns_t **tns);
ZAPI zerr_t ztns_destroy(ztns_t *tns);
ZAPI zerr_t ztns_init(ztns_t *tns);
ZAPI zerr_t ztns_fini(ztns_t *tns);

ZAPI zerr_t ztns_connect(ztns_t *tns, zsa_t *addr, ztns_on_event onconn);
ZAPI zerr_t ztns_listen(ztns_t *tns, zsa_t *addr, zsa_len_t len, ztns_on_event onaccept);
ZAPI zerr_t ztns_send(ztns_conn_t *conn, ztns_pkt_t *pkt);
ZAPI zerr_t ztns_recv(ztns_conn_t *conn, ztns_pkt_t **pkt);
ZAPI zerr_t ztns_close(ztns_conn_t *conn);
/*
 * packet manager APIs
 */
ZAPI ztns_pkt_t *ztns_pkt_alloc(ztns_conn_t *conn); /* size = ZTNS_PKT_SIZE = 16384 */
ZAPI void ztns_pkt_refer(ztns_pkt_t *pkt); /* add packet refers*/
ZAPI ztns_pkt_t *ztns_pkt_clone(ztns_pkt_t *pkt); /* deep copy packet */
ZAPI void ztns_pkt_free(ztns_pkt_t *pkt);
ZAPI ztns_pkt_t *ztns_pkt_write(ztns_pkt_t *pkt, zptr_t data, size_t len);
ZAPI ztns_pkt_t *ztns_pkt_data(ztns_pkt_t *pkt, zptr_t *buf, size_t *len);
ZAPI zerr_t ztns_pkt_dump(ztns_pkt_t *pkt);
/* store and fetch pollin packets
 * It's useful for a message that needs call ztns_recv() many times.
 * for example a message has 4KB, receive 2KB one time, so needs call 2 times.
 * ztns_pkt_t *pkt = ztns_pkt_fetch(conn);
 * ztens_recv(conn, pkt); // read 2KB
 * ztns_pkt_store(conn, pkt); // store pkt, for next fetch
 * // on next pollin call back
 * pkt = ztns_pkt_fetch(conn);
 * zten_recv(conn, pkt);
 * // do something on packet, after read the hole message
 * ztns_pkt_free(pkt);
 */
ZAPI ztns_pkt_t *ztns_pkt_fetch(ztns_conn_t *conn);
ZAPI void ztns_pkt_store(ztns_conn_t *conn, ztns_pkt_t *pkt);

/**
 * @brief splice packet list to packet
 * @remark It will cause memory copy and memory allocate, try avoid to use it.
 */
ZAPI zerr_t ztns_pkt_splice(ztns_pkt_t *pkt, ztns_pkt_t **spliced_pkt);
ZC_END

#endif /*_Z_TRANSPORT_H_*/
