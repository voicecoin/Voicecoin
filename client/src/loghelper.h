#ifndef _BCUS_LOG_HELPER_H_
#define _BCUS_LOG_HELPER_H_

#include <string>
#include <cstddef>
#include <stdio.h>
#include <stdarg.h>
#include "file.h"
#include "platform.h"
#include <typeinfo>
#include "singleton.h"


typedef enum {
    XLOG_FATAL      =   0X01,
    XLOG_ERROR      =   0X02,
    XLOG_WARNING    =   0X04,
    XLOG_INFO       =   0X08,
    XLOG_DEBUG      =   0X10,
    XLOG_TRACE      =   0X20,
    XLOG_ALL        =   0XFF
} XLOG_LEVEL;


class xlog_helper  {
public:
    static void set_log_file(const char *filename);
    static inline void set_log_level(int level) { loglevel_ = level; }
    static void set_log_level(const char *strlevel);
    static int  get_log_level() {return loglevel_;}
    static void log(int loglevel, const char *fmt, ...);
public:
    static int loglevel_;
private:
    static FILE *log_file_;
};


#define SET_LOG_FILE(file) xlog_helper::set_log_file(file)
#define SET_LOG_LEVEL(level) xlog_helper::set_log_level(level)

//void XLOG(int loglevel, const char *fmt, ...);
#define XLOG(level, fmt, ...)                                                       \
    do                                                                              \
        if (level == (level & xlog_helper::loglevel_)) {                            \
            xlog_helper::log(level, fmt, ## __VA_ARGS__);                           \
        }                                                                           \
    while(0)

static inline char get_visible_ascii(char c) { return (c >= 33 && c <= 126) ? c : '.'; }
static std::string binary_dump_string(const char *buf, int len, int indent = 4) {
    if (len <= 0) {
        return "";
    }
    std::string info;
    std::string strindent(indent, ' ');

    int line = len >> 4;
    int last = (len & 0xF);
    int i = 0;
    for (i = 0; i < line; ++i) {
        char szbuf[256] = {0};
        const unsigned char * base = (const unsigned char *)(buf + (i << 4));
        sprintf(szbuf,"%s[0X%04X]  %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x %02x%02x   %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c\n",
            strindent.c_str(), i * 16, *(base),*(base+1),*(base+2),*(base+3),*(base+4),*(base+5),*(base+6),*(base+7),
            *(base + 8),*(base+9),*(base+10),*(base+11),*(base+12),*(base+13),*(base+14),*(base+15),
            get_visible_ascii(*(base)), get_visible_ascii(*(base+1)), get_visible_ascii(*(base+2)), get_visible_ascii(*(base+3)),
            get_visible_ascii(*(base+4)), get_visible_ascii(*(base+5)), get_visible_ascii(*(base+6)), get_visible_ascii(*(base+7)),
            get_visible_ascii(*(base+8)), get_visible_ascii(*(base+9)), get_visible_ascii(*(base+10)), get_visible_ascii(*(base+11)),
            get_visible_ascii(*(base+12)), get_visible_ascii(*(base+13)), get_visible_ascii(*(base+14)), get_visible_ascii(*(base+15)));
        info.append(szbuf);
    }

    if (last > 0) {
        const unsigned char * base = (const unsigned char *)(buf + (i << 4));
        char szhex[64] = {0};
        char szascii[64] = {0};
        int num = last >> 1; // last / 2
        int j = 0;
        for(; j < num; ++j) {
            sprintf(szhex + j * 5, " %02x%02x", *(base + 2*j), *(base + 2*j + 1));
            sprintf(szascii + j * 3, " %c%c", get_visible_ascii(*(base + 2*j)), get_visible_ascii(*(base + 2*j + 1)));
        }
        if ((last & 0X01) == 1) {
            sprintf(szhex + j * 5, " %02x", *(base + last-1));
            sprintf(szascii + j * 3, " %c", get_visible_ascii(*(base + last-1)));
        }
        char szbuf[128] = {0};
        sprintf(szbuf, "%s[0X%04X] %-40s  %s", strindent.c_str(), i*16, szhex, szascii);
        info.append(szbuf);
        info.append(1,'\n');
    }
    return info;
}
static inline std::string binary_to_string(const char *buf, int len)
{
    char sz[256] = {0};
    const unsigned char *p = (const unsigned char *)buf;
    int n = 0;
    int i = 0;
    for (; i + 1 < len; i += 2) {
        n += snprintf(sz + n, sizeof(sz) - n, "%02X%02X ", *(p + i), *(p + i + 1));
    }
    if (i % 2 == 1) {
        snprintf(sz + n, sizeof(sz) - n, "%02X ", *(p + i));
    }
    return sz;
}

#endif
