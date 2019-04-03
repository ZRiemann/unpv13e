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
 * @file transport_server.c
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2019-04-03 Z.Riemann found
 */

#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <zsi/app/waitloop.h>
#include <zsi/app/trace2console.h>
#include <znt/sock/transport.h>

static zerr_t on_pollin(ztns_t *tns, ztns_conn_t *conn){
    zerr_t ret = ZEOK;
    // receive data
    ztns_pkt_t *pkt = ztns_pkt_fetch(conn);
    if(!pkt){
        zerrno(ZEMEM_INSUFFICIENT);
        return ZEMEM_INSUFFICIENT;
    }

    if(ZEOK == (ret = ztns_recv(conn, pkt))){
        ztns_pkt_dump(pkt);
#if 0
        // if packet needs read more data.
        // it will be fetched on next pollin by call ztns_pkt_fetch(conn)
        ztns_pkt_store(conn, pkt);
#else
        // echo the received packet
        ztns_send(conn, ztns_pkt_t pkt); // echo back, pkt will recycle after send.
#endif
    }else{
        ztns_pkt_dump(pkt);
        ztns_pkt_free(pkt); // if not send, just free it.
    }
    return ret;
}

static zerr_t on_accept(ztns_t *tns, ztns_conn_t *conn){
    // verify connection
    // just accept all connections
    conn->on_event = on_pollin;
    return ZEOK;
}

int main(int argc, char **argv){
    ztns_t *tns;
    struct sockaddr_in	servaddr;

    ztrace_register(ztrace2console, NULL);
    zmsg("transport_server...");
    ztns_create(&tns);
    ztns_init(tns);

    servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(13000);

    ztns_listen(tns,(zsa_t*)servaddr, sizeof(servaddr), on_accept);
    zwait_exit_signal();

    ztns_fini(tns);
    ztns_destroy(tns);
    zmsg("testing done.");
    return ZEOK;

}