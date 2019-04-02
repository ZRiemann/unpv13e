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
#ifndef _ZNT_SESSION_H_
#define _ZNT_SESSION_H_

/**
 * @file session.h
 * @brief <A brief description of what this file is.>
 * @author Z.Riemann https://github.com/ZRiemann/
 * @date 2019-04-01 Z.Riemann found
 * @par Synopsys
 *  -# session timeout
 *  -# auto reconnection
 *  -# heart beat
 *  -# load balancing
 *  -# session modes pub/sub push/pull request/response
 *  -# broadcast/multicast
 *  -# messages storage
 *  -# network monitor
 */
#include <znt/sock/transport.h>

ZC_BEGIN

typedef struct zsession_s{
    ztns_conn_t *load_balances;
}zssn_t;

/**
 * @brief ztns_conn_t.extension
 */
typedef struct zsession_control_s{
    int heartbeat_interval; /** seconds */
}zssn_ctl_t;

/**
 * 
 */
typedef struct zsession_manager_s{
    
}zssn_mgr_t;

ZC_END
#endif /*_ZNT_SESSION_H_*/
