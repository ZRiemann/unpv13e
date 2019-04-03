#include <zsi/base/error.h>
#include <znt/sock/sock.h>

#define ZTRACE_TITLE "ZNT-SOCK"
#define ZTRACE_LEVEL_CTL 0x1f
#include <zsi/base/trace.h>

zfd_t zsocket(int family, int type, int protocol){
    zfd_t fd = socket(family, type, protocol);
    if(fd < 0){
        zerrno(errno);
        fd = ZSOCK_INVALID;
    }else{
        zmsg("create fd: %d", fd);
    }
    return fd;
}

void zsock_close(zfd_t fd){
    if(-1 == close(fd)){
        zerrno(ZESOCK_INVALID);
    }else{
        zmsg("close fd: %d", fd);
    }
}

zerr_t zbind(zfd_t fd, SA *addr, socklen_t len){
    if(bind(fd, addr, len) < 0){
        zerrno(errno);
        return errno;
    }
    zmsg("bind(fd<%d>)", fd);
    return ZEOK;
}
zerr_t zlisten(zfd_t fd, int backlog){
    char *ptr;
    /*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL){
        backlog = atoi(ptr);
    }

	if (listen(fd, backlog) < 0){
        zerrno(errno);
        return errno;
    }
    zmsg("listen(fd<%d>, backlog<%d>)", fd, backlog);
    return ZEOK;
}

/**
 * accetp extracts the first connection request on the queue of pending connec-
 * tions for the listenning socket.(SOCK_STREAM, SOCK_SEQPACK)
 * @param fd [in] a socket bind(2) and listen(2)
 * @param src [out] the address of the peer socket, NULL
 * @param len [in|out] sizeof <src>
 * @param flag [in, out] accept4(2) 0|SOCK_NONBLOCK|SOCK_CLOEXEC, out of EAGAIN|0
 * @return -1 on error
 *         >0 a file descriptor(linux not inferit O_NONBLOCK|O_ASYNC from <fd>)
 */
zfd_t zaccept(zfd_t fd, SA* src, socklen_t *len, int *flag){
    zfd_t _fd;
again:
    if ( (_fd = accept4(fd, src, len, *flag)) < 0){
        zerr_t err = errno;
        zerrno(err);
#ifdef EPROTO // apuev13e $5.11
        if(EPROTO == err || ECONNABORTED == err){
            goto again;
        }
#else
        if(ECONNABORTED == err){
            goto again;
        }
#endif //EPROTO
        _fd = ZSOCK_INVALID;
        *flag = err == EAGAIN ? EAGAIN : 0;
    }
    return _fd;
}

/* numeric to presentation */
zerr_t zsock_ntop(char* str, size_t len,
                  const SA *sa, socklen_t salen){
    char port[8];
    switch(sa->sa_family){
    case AF_INET:{
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
        if(inet_ntop(AF_INET, &sin->sin_addr, str, len) == NULL){
            zerrno(ZEPARAM_INVALID);
            return ZEPARAM_INVALID;
        }
        if(sin->sin_port != 0){
            snprintf(port, sizeof(port), ":%d", ntohs(sin->sin_port));
            strcat(str, port);
        }
        return ZEOK;
    }
#ifdef IPV6
    case AF_INET6:{
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6*) sa;
        if(inet_ntop(AF_INET6, &sin6->sin6_addr, str, len) == NULL){
            zerrno(ZEPARAM_INVALID);
            return ZEPARAM_INVALID;
        }
        if(sin6->sin6_port){
            snprintf(port, sizeof(port), ":%d", ntohs(sin6->sin6_port));
            strcat(str, port);
        }
        return ZEOK;
    }
#endif
#ifdef AF_UNIX
    case AF_UNIX:{
        struct sockaddr_un *unp = (struct sockaddr_un*) sa;
        if(unp->sun_path[0] == 0){
            strcpy(str, "(no pathname bound)");
        }else{
            snprintf(str, len, "%s", unp->sun_path);
        }
        return ZEOK;
    }
#endif
#ifdef HAVE_SOCKADDR_DL_STRUCT
    case AF_LINK:{
        struct sockaddr_dl *sdl = (struct sockaddr_dl *) sa;
        if(sdl->sdl_nlen > 0){
            snprintf(str, len, "%*s (index %d)",
                     sdl->sdl_nlen, &sdl->sdl_data[0], sdl->sdl_index);
        }else{
            snprintf(str, len, "AF_LINK, index=%d", sdl->sdl_index);
        }
        return ZEOK;
    }
#endif
    default:
        snprintf(str, len, "unknown AF_xxx: %d, len %d", sa->sa_family, salen);
    }
    return ZENOT_SUPPORT;
}
/* presentation to numeric */
zerr_t zsock_pton(SA *sa, socklen_t addrlen,
                  const char *str){
    return ZENOT_SUPPORT;
}
/**
 * @brief 绑定到通配地址
 * @param [in] fd
 * @param [in] family
 * @param [in] port 指定端口，if 0=*port 随机端口
 *        [out]     返回绑定的端口
 * @retval ZEOK ok
 */
zerr_t zsock_bind_wild(zfd_t sockfd, int family, zport_t *port){
    socklen_t	len;

	switch (family) {
	case AF_INET: {
		struct sockaddr_in	sin;

		bzero(&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = htonl(INADDR_ANY);
		sin.sin_port = htons(*port);	/* bind ephemeral port */

		if (bind(sockfd, (SA *) &sin, sizeof(sin)) < 0){
            zerrno(errno);
            return errno;
        }
		len = sizeof(sin);
		if (getsockname(sockfd, (SA *) &sin, &len) < 0){
            zerrno(errno);
            return errno;
        }
        *port = sin.sin_port;
		return ZEOK;
	}

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	sin6;

		bzero(&sin6, sizeof(sin6));
		sin6.sin6_family = AF_INET6;
		sin6.sin6_addr = in6addr_any;
		sin6.sin6_port = htons(*port);	/* bind ephemeral port */

		if (bind(sockfd, (SA *) &sin6, sizeof(sin6)) < 0){
            zerrno(errno);
            return errno;
        }
		len = sizeof(sin6);
		if (getsockname(sockfd, (SA *) &sin6, &len) < 0){
            zerrno(errno);
            return errno;
        }
        *port = sin6.sin6_port;
		return ZEOK;
	}
#endif
	}
    zerrno(ZEPARAM_INVALID);
	return ZEPARAM_INVALID;
}

/**
 * @brief read <n> bytes from <fd>
 * @param [in] fd socket filedescriptor created by socket
 * @param [out] buf buffer to write
 * @param [in] *n buffer size
 *        [out] *n bytes already readed
 * @retval ZEOK <n> bytes teaded
 * @retval ZE_EOF <fd> closed by peer
 * @retval errno  other socket errors
 */
zerr_t zreadn(zfd_t fd, void *buf, size_t *n){
    size_t nleft = *n;
    ssize_t nread;
    char *ptr = buf;

    while(nleft > 0){
        if((nread = read(fd, ptr, nleft)) < 0){
            if(errno == EINTR){
                nread = 0; /* interrupted, call again */
            }else{
                zerrno(errno);
                return errno;
            }
        }else if(0 == nread){
            break; /* EOF */
        }
        nleft -= nread;
        ptr += nread;
    }
    *n -= nleft;
    return nleft ? ZE_EOF : ZEOK;
}

/**
 * @brief write <*n> bytes to <fd>
 * @param [in] fd socket file descriptor created by socket
 * @param [in] buf The data to be write
 * @param [in] *n bytes to be write
 *        [out] *n bytes written
 * @retval ZEOK <*n> bytes written
 * @retval errno socket errors
 */
zerr_t zwritten(zfd_t fd, const void *buf, size_t *n){
    size_t nleft = *n;
    ssize_t nwritten = 0;
    const char *ptr = buf;

    while(nleft > 0){
        if((nwritten = write(fd, ptr, nleft)) <=0){
            if(nwritten < 0 && errno == EINTR){
                nwritten = 0;
            }else{
                zerrno(errno);
                return errno == EAGAIN ? ZEAGAIN : ZEFAIL;
            }
            nleft -= nwritten;
            ptr += nwritten;
        }
    }
    return ZEOK;
}

/**
 * @brief read one line from fd
 * @param [in] fd Socket file descriptor
 * @param [out] buf The line buffer
 * @param [in] *n The line buffer size
 *        [out] *n The line buffer readed
 * @retval ZEOK read one line
 */
zerr_t zreadline(zfd_t fd, void *buf, size_t *n){
    return ZENOT_SUPPORT;
}