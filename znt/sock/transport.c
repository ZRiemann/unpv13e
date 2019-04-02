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
#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <znt/sock/transport.h>

static zbuf_t *g_buf;
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
    if(NULL == (head = zbuf_alloc(g_buf, sizeof(ztns_t)))){
        zerrno(ZEMEM_INSUFFICIENT);
        return ZEMEM_INSUFFICIENT;
    }
    *tns = (ztns_t*)head->payload;
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
    return ZEOK;
}
/**
 * @brief startup transport server
 * @param [in] tns Transport handle
 */
zerr_t ztns_init(ztns_t *tns){
    return ZENOT_SUPPORT;
}
/**
 * @brief stop transport server
 * @param [in] tns Transport handle
 */
zerr_t ztns_fini(ztns_t *tns){
    return ZENOT_SUPPORT;
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
    return ZENOT_SUPPORT;
}
/**
 * @brief startup passive connection
 * @param [in] tns The transport handle
 * @param [in] addr peer connection address
 * @param [in] onaccept new connection callback
 * @param ZEOK
 */
zerr_t ztns_listen(ztns_t *tns, zsa_t *addr, ztns_on_accept onaccept){
    return ZENOT_SUPPORT;
}
/**
 * @brief send a packet
 * @param [in] conn connection handle
 * @param [in] pkt send buffer
 * @retval ZEOK
 */
zerr_t ztns_send(ztns_conn_t *conn, ztns_packet_t *pkt){
    return ZENOT_SUPPORT;
}
/**
 * @brief recveive a packet(list)
 * @param [in] conn connection handle
 * @param [out] pkt receive buffer
 */
zerr_t ztns_recv(ztns_conn_t *conn, ztns_packet_t *pkt){
    return ZENOT_SUPPORT;
}
/**
 * @brief Close the connection
 * @param conn Connection handle to be closed
 * @retval ZEOK
 */
zerr_t ztns_close(ztns_conn_t *conn){
    return ZENOT_SUPPORT;
}

/**
 * @brief alloc a packet(list)
 * @param head packet list header, if NULL it will alloc single packet
 * @return the new packet and append to list tail
 * @remark size = ZTNS_PKT_SIZE = 16384
 */
ztns_pkt_t *ztns_pkt_alloc(ztns_pkt_t *head){
    return NULL;
}
/**
 * @brief add refers to packet(list)
 */
void ztns_pkt_refer(ztns_pkt_t *pkt){
    reutrn ZENOT_SUPPORT;
}
/**
 * @brief deep copy packet(list)
 */
znts_pkt_t *ztns_pkt_clone(ztns_pkt_t *pkt){
    return ZENOT_SUPPORT;
}
/**
 * @brief recycle the packet(list)
 */
void ztns_pkt_free(ztns_pkt_t *pkt){

}
/**
 * @brief write <data> to <pkt> list
 */
ztns_pkt_t *ztns_pkt_write(ztns_pkt_t *pkt, zptr_t data, size_t len){
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
    return ZENOT_SUPPORT;
}
