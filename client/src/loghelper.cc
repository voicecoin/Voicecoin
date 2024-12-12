#include "loghelper.h"
#include <cstring>
//#include <bcus/system/mutex.h>
#include <boost/smart_ptr/detail/spinlock.hpp>
#if defined(LINUX) || defined(MAC)
#include <sys/syscall.h>
#endif

int xlog_helper::loglevel_   = XLOG_ALL;
FILE *xlog_helper::log_file_ = stdout;
void xlog_helper::set_log_file(const char *filename)
{
    if (filename == NULL || *filename == 0) {
        return;
    }

    bcus::file f;
    f.open(filename, "a+", true);
    log_file_ = f.get();
}
void xlog_helper::set_log_level(const char *strlevel) {
    if (0 == strcasecmp(strlevel, "trace")) {
        loglevel_ = XLOG_FATAL | XLOG_ERROR | XLOG_WARNING | XLOG_INFO | XLOG_DEBUG | XLOG_TRACE;
    } else if (0 == strcasecmp(strlevel, "debug")) {
        loglevel_ = XLOG_FATAL | XLOG_ERROR | XLOG_WARNING | XLOG_INFO | XLOG_DEBUG;
    } else if (0 == strcasecmp(strlevel, "info")) {
        loglevel_ = XLOG_FATAL | XLOG_ERROR | XLOG_WARNING | XLOG_INFO;
    } else if (0 == strcasecmp(strlevel, "warning")) {
        loglevel_ = XLOG_FATAL | XLOG_ERROR | XLOG_WARNING;
    } else if (0 == strcasecmp(strlevel, "error")) {
        loglevel_ = XLOG_FATAL | XLOG_ERROR;
    } else if (0 == strcasecmp(strlevel, "fatal")) {
        loglevel_ = XLOG_FATAL;
    } else if (0 == strcasecmp(strlevel, "all")) {
        loglevel_ = XLOG_FATAL | XLOG_ERROR | XLOG_WARNING | XLOG_INFO | XLOG_DEBUG | XLOG_TRACE;
    } else {
        fprintf(stderr, "Bad LogLevel[%s]\n", strlevel);
    }
}

static inline const char *str_log_level(int level) {
    static const char *LOG_LEVEL[0X21] = {
        "0",                                                /* skip 0                       */
        "FATAL",                                            /* XLOG_FATAL      =   0X01,    */
        "ERROR",                                            /* XLOG_ERROR      =   0X02,    */
        "3",                                                /* skip 3                       */
        "WARNING",                                          /* XLOG_WARNING    =   0X04,    */
        "5", "6", "7",                                      /* skip 5 6 7                   */
        "INFO",                                             /* XLOG_INFO       =   0X08,    */
        "9", "10", "11", "12", "13", "14", "15",            /* skip 9 10 11 12 13 14 15     */
        "DEBUG",                                            /* XLOG_DEBUG       =   0X10,   */
        "17", "18", "19", "20", "21", "22", "23",           /* skip 17 18 19 20 21 22 23    */
        "24", "25", "26", "27", "28", "29", "30", "31",     /* skip 24 25 26 27 28 29 30 31 */
        "TRACE"                                             /* XLOG_TRACE       =   0X20,   */
    };
    return LOG_LEVEL[level];
}

void xlog_helper::log(int loglevel, const char *fmt, ...) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm local_tm;
    localtime_r(&(tv.tv_sec), &local_tm);

    char szthreadid[32] = {0};
#if defined(WINDOWS) || defined(WIN32)
    snprintf(szthreadid, sizeof(szthreadid), "%lu", GetCurrentThreadId());
#elif defined(LINUX)
    snprintf(szthreadid, sizeof(szthreadid), "%lu", syscall(SYS_gettid));
#elif defined(MAC)
    snprintf(szthreadid, sizeof(szthreadid), "%llu", (uint64_t)pthread_self());
#endif

    //static bcus::system::spinlock _spinlock;
    static boost::detail::spinlock _spinlock;
    _spinlock.lock();

    fprintf(log_file_, "[%s] %04d-%02d-%02d %02d:%02d:%02d %03d.%03d %s ", szthreadid, local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
        local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, (int)(tv.tv_usec/1000), (int)(tv.tv_usec % 1000),
        str_log_level(loglevel));

    va_list va;
    va_start( va, fmt );
    vfprintf( log_file_, fmt, va );
    va_end( va );

    fflush(log_file_);
    _spinlock.unlock();

}
