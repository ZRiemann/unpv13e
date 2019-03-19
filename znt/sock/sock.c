#include <zsi/base/error.h>
#include <zsi/base/trace.h>
#include <znt/sock/sock.h>

// redifine trace function, add title
#define UNP_TITLE "ZNT-SOCK"
#define UNP_LEVEL_CTL 0
#undef zdbg
#undef zmsg
#undef zwar
#undef zerr
#undef zinf
#undef zerrno

#if ZTRACE_WITH_FILE
#define zdbg(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_DBG, UNP_TITLE, "[%03d %s@%s]\t" fmt,\
                              __LINE__, __FUNCTION__, __FILE__, ##__VA_ARGS__)
#define zmsg(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_MSG, UNP_TITLE, "[%03d %s@%s]\t" fmt,\
                              __LINE__, __FUNCTION__, __FILE__, ##__VA_ARGS__)
#define zwar(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_WAR, UNP_TITLE, "[%03d %s@%s]\t" fmt,\
                              __LINE__, __FUNCTION__, __FILE__, ##__VA_ARGS__)
#define zerr(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_ERR, UNP_TITLE, "[%03d %s@%s]\t" fmt,\
                              __LINE__, __FUNCTION__, __FILE__, ##__VA_ARGS__)
#define zinf(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_INF, UNP_TITLE, "[%03d %s@%s]\t" fmt,\
                              __LINE__, __FUNCTION__, __FILE__, ##__VA_ARGS__)
#define zerrno(errno) if(ZEOK != errno){\
    ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_ERR, UNP_TITLE, "[%03d %s@%s]\t %s", \
           __LINE__, __FUNCTION__, __FILE__, zstrerr(errno));}
#else
#define zdbg(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_DBG, UNP_TITLE, "[%03d %s]\t" fmt,\
                              __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zmsg(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_MSG, UNP_TITLE, "[%03d %s]\t" fmt,\
                              __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zwar(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_WAR, UNP_TITLE, "[%03d %s]\t" fmt,\
                              __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zerr(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_ERR, UNP_TITLE, "[%03d %s]\t" fmt,\
                              __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zinf(fmt, ...) ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_INF, UNP_TITLE, "[%03d %s]\t" fmt,\
                              __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define zerrno(errno) if(ZEOK != errno){\
    ztrace(UNP_LEVEL_CTL, ZTRACE_LEVEL_ERR, UNP_TITLE, "[%03d %s]\t %s", \
           __LINE__, __FUNCTION__, zstrerr(errno));}
#endif

zfd_t zsocket(int family, int type, int protocol){
    zfd_t fd = socket(family, type, protocol);
    if(fd < 0){
        zerrno(ZESOCK_INVALID);
    }else{
        zdbg("create fd: %d", fd);
    }
    return fd;
}

void zsock_close(zfd_t fd){
    if(-1 == close(fd)){
        zerrno(ZESOCK_INVALID);
    }else{
        zdbg("close fd: %d", fd);
    }
}

zerr_t zbind(zfd_t fd, const char *addr){
    zerrno(ZENOT_SUPPORT);
    return ZEOK;
}
zerr_t zlisten(zfd_t fd, int queue_size){
    zerrno(ZENOT_SUPPORT);
    return ZEOK;
}
zerr_t zaccept(zfd_t fd, SA* src, socklen_t *len){
    zerrno(ZENOT_SUPPORT);
    return ZEOK;
}
