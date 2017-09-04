#ifndef BCUS_PLATFORM_H
#define BCUS_PLATFORM_H

#if defined(__GNUC__)
#  ifndef GCC_VERSION__
#   define GCC_VERSION__ (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#  endif
#endif

#define BCUS_ESC(...) __VA_ARGS__

//#if defined( WIN32) || defined(WINDOWS)
#if defined( _MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#  include <winsock2.h>
#  include <time.h>
#  include <sys/timeb.h>
#  include <sys/stat.h>
#  include <direct.h>
#  include <io.h>
#  include <stdint.h>
#  include <Windows.h>
#  include <Iphlpapi.h>
#  include <signal.h>
#  include <float.h>
#  include <math.h>
#  include <stdlib.h>
#  include <stdio.h>

#  define BCUS_LOG_DIRSPLITOR '\\'
#  define access _access
#  define mkdir _mkdir
inline void sleep(int sec) { Sleep((sec) * 1000); }
#  define usleep(usec) Sleep((usec) / 1000000)
#  define gmtime_r(A, B) gmtime_s(B, A)

// vs2015 has snprintf
#if _MSC_VER < 1900
#  ifndef snprintf
#  define snprintf _snprintf
#  endif

   inline long long atoll (const char *p) {
        int minus = 0;
        long long value = 0;
        if (*p == '-') {
            minus ++;
            p ++;
        }
        while (*p >= '0' && *p <= '9') {
            value *= 10;
            value += *p - '0';
            p ++;
        }
        return minus ? 0 - value : value;
    }
#endif


#  ifndef isnan
#  define isnan _isnan
#  endif

#  ifndef strcasecmp
#  define strcasecmp stricmp
#  endif

#  ifndef strncasecmp
#  define strncasecmp strnicmp
#  endif

#  ifndef ftruncate
#  define ftruncate chsize
#  endif

#  ifndef stat
#  define stat _stati64
#  endif

#  ifndef unlink
#  define unlink _unlink
#  endif


   int inline gettimeofday (struct timeval *tp, void *tz) {
        struct _timeb timebuffer;
        _ftime64_s(&timebuffer);
        tp->tv_sec  = (uint64_t)(timebuffer.time);
        tp->tv_usec = timebuffer.millitm * 1000;
        return 0;
   }
//#  define localtime_r(a, b) localtime_s((const time_t *)(a), b)
#  define localtime_r(a, b)  _localtime32_s(b, (const __time32_t *)(a))

    /* Signal types defined by windows */
    //#define SIGINT          2       /* interrupt */
    //#define SIGILL          4       /* illegal instruction - invalid function image */
    //#define SIGFPE          8       /* floating point exception */
    //#define SIGSEGV         11      /* segment violation */
    //#define SIGTERM         15      /* Software termination signal from kill */
    //#define SIGBREAK        21      /* Ctrl-Break sequence */
    //#define SIGABRT         22      /* abnormal termination triggered by abort call */

    #define SIGHUP      -1
    #define SIGQUIT     -1
    #define SIGTRAP     -1
    #define SIGBUS      -1
    #define SIGKILL     -1
    #define SIGUSR1     -1
    #define SIGUSR2     -1
    #define SIGPIPE     -1
    #define SIGALRM     -1
    #define SIGSTKFLT   -1
    #define SIGCONT     -1
    #define SIGSTOP     -1
    #define SIGTSTP     -1
    #define SIGTTIN     -1
    #define SIGTTOU     -1
    #define SIGURG      -1
    #define SIGXCPU     -1
    #define SIGXFSZ     -1
    #define SIGVTALRM   -1
    #define SIGPROF     -1
    #define SIGWINCH    -1
    #define SIGIO       -1
    #define SIGPWR      -1
    #define SIGSYS      -1
#else  //defined( WIN32) || defined(WINDOWS)

#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#  include <cerrno>
#  include <sys/time.h>
#  include <pthread.h>
#  include <dirent.h>
#  include <fnmatch.h>
#  include <stdio.h>
#  include <stdint.h>
#  include <signal.h>
#  include <string.h>
#  include <float.h>
#  include <stdlib.h>

#  ifdef MAC
    typedef struct __siginfo siginfo_t;
#  endif

#  define BCUS_LOG_DIRSPLITOR '/'

#  ifndef MAX_PATH
#    define MAX_PATH 256
#  endif

#endif //defined( WIN32) || defined(WINDOWS)


/** signal function adapter **/
namespace bcus {
    typedef void (*bcus_signal_handler)(int);
    static inline void signal(int sig, bcus_signal_handler h) {
        if (sig == -1) {
            return;
        }
        ::signal(sig, h);
    }
}

/** float defination */
#ifndef DBL_MAX
#define DBL_MAX __DBL_MAX__
#endif

#ifndef BCUS_DOUBLE_NULL
#define BCUS_DOUBLE_NULL DBL_MAX
#endif

#ifndef DOUBLE_NaN
#define DOUBLE_NaN DBL_MAX
#endif



/** define BYTE_SWAP **/
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# include <stdlib.h>
# define BYTE_SWAP_U16(x) _byteswap_ushort(x)
# define BYTE_SWAP_U32(x) _byteswap_ulong(x)
# define BYTE_SWAP_U64(x) _byteswap_uint64(x)
#elif defined(__APPLE__)
# include <libkern/OSByteOrder.h>
# define BYTE_SWAP_U16(x) OSSwapInt16(x)
# define BYTE_SWAP_U32(x) OSSwapInt32(x)
# define BYTE_SWAP_U64(x) OSSwapInt64(x)
#elif defined(GCC_VERSION__) && (GCC_VERSION__ >= 40300)
# define BYTE_SWAP_U16(x) (uint16_t)((x) << 8 | (x) >> 8)
# define BYTE_SWAP_U32(x) (uint32_t)(__builtin_bswap32(x))
# define BYTE_SWAP_U64(x) (uint64_t)(__builtin_bswap64(x))
#else

# ifndef UINT64_C
# define UINT64_C(c) (c##ULL)
# endif

# define BYTE_SWAP_U16(x) (uint16_t)((x) << 8 | (x) >> 8)
# define BYTE_SWAP_U32(x) (uint32_t)(((x) << 24) | \
    (((x) <<  8) & 0x00FF0000) | \
    (((x) >>  8) & 0x0000FF00) | ((x) >> 24))
# define BYTE_SWAP_U64(x) \
    ( (((x) & UINT64_C(0xff00000000000000)) >> 56) \
    | (((x) & UINT64_C(0x00ff000000000000)) >> 40) \
    | (((x) & UINT64_C(0x0000ff0000000000)) >> 24) \
    | (((x) & UINT64_C(0x000000ff00000000)) >> 8)  \
    | (((x) & UINT64_C(0x00000000ff000000)) << 8)  \
    | (((x) & UINT64_C(0x0000000000ff0000)) << 24) \
    | (((x) & UINT64_C(0x000000000000ff00)) << 40) \
    | (((x) & UINT64_C(0x00000000000000ff)) << 56) )
#endif


inline double BYTE_SWAP_DOUBLE(double d) {
    uint64_t tmp = BYTE_SWAP_U64(*((uint64_t *)((void*)&(d))));
    double ret;
    *((uint64_t*)&ret) = tmp;
    return ret;
}


/** detach byte order **/
#ifdef LINUX
#  include <endian.h>
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#    define BCUS_IS_LITTLE_ENDIAN 1
#  else
     define BCUS_IS_LITTLE_ENDIAN 0
#  endif
#elif defined(MAC)
#  include <sys/_endian.h>
#  if __DARWIN_BYTE_ORDER == __DARWIN_LITTLE_ENDIAN
#    define BCUS_IS_LITTLE_ENDIAN 1
#  else
     define BCUS_IS_LITTLE_ENDIAN 0
#  endif
#elif defined(_MSC_VER)
#    define BCUS_IS_LITTLE_ENDIAN 1
#endif

#define BCUS_USE_LITTLE_ENDIAN

// Macro-version's byte sequence swapping
#ifdef BCUS_USE_LITTLE_ENDIAN  //defined in Makefile
#  if BCUS_IS_LITTLE_ENDIAN
#    define g_htons(x)  (x)
#    define g_htonl(x)  (x)
#    define g_htonll(x) (x)
#    define g_htond(x)  (x)
#    define g_ntohs(x)  (x)
#    define g_ntohl(x)  (x)
#    define g_ntohll(x) (x)
#    define g_ntohd(x)  (x)
#  else
#    define g_htons(x)  BYTE_SWAP_U16((uint16_t)(x))
#    define g_htonl(x)  BYTE_SWAP_U32((uint32_t)(x))
#    define g_htonll(x) BYTE_SWAP_U64((uint64_t)(x))
#    define g_htond(x)  BYTE_SWAP_DOUBLE(x)
#    define g_ntohs(x)  BYTE_SWAP_U16((uint16_t)(x))
#    define g_ntohl(x)  BYTE_SWAP_U32((uint32_t)(x))
#    define g_ntohll(x) BYTE_SWAP_U64((uint64_t)(x))
#    define g_htond(x)  BYTE_SWAP_DOUBLE(x)
#  endif
#else //defined(BCUS_USE_BIG_ENDIAN)
#  if BCUS_IS_LITTLE_ENDIAN
#    define g_htons(x)  BYTE_SWAP_U16((uint16_t)(x))
#    define g_htonl(x)  BYTE_SWAP_U32((uint32_t)(x))
#    define g_htonll(x) BYTE_SWAP_U64((uint64_t)(x))
#    define g_htond(x)  BYTE_SWAP_DOUBLE(x)
#    define g_ntohs(x)  BYTE_SWAP_U16((uint16_t)(x))
#    define g_ntohl(x)  BYTE_SWAP_U32((uint32_t)(x))
#    define g_ntohll(x) BYTE_SWAP_U64((uint64_t)(x))
#    define g_ntohd(x)  BYTE_SWAP_DOUBLE(x)
#  else
#    define g_htons(x)  (x)
#    define g_htonl(x)  (x)
#    define g_htonll(x) (x)
#    define g_htond(x)  (x)
#    define g_ntohs(x)  (x)
#    define g_ntohl(x)  (x)
#    define g_ntohll(x) (x)
#    define g_ntohd(x)  (x)
#  endif
#endif//BCUS_USE_LITTLE_ENDIAN


/** define host net byte order converters **/
//static union {char c[4]; unsigned long mylong;} endian_test = {{'l','?','?','b'}}; //{{0X01,0X00,0X00,0X02}};
//#define ENDIANNESS ((char)endian_test.mylong)
//#define _IS_LITTLE_ENDIAN_ (ENDIANNESS == 'l')
/*
# define g_htons(x)  (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U16((uint16_t)(x)) : (x))
# define g_htonl(x)  (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U32((uint32_t)(x)) : (x))
# define g_htonll(x) (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U64((uint64_t)(x)) : (x))
# define g_htond(x)  (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U64(*(uint64_t *)(&x)) : (x))
# define g_ntohs(x)  (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U16((uint16_t)(x)) : (x))
# define g_ntohl(x)  (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U32((uint32_t)(x)) : (x))
# define g_ntohll(x) (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U64((uint64_t)(x)) : (x))
# define g_ntohd(x)  (_IS_LITTLE_ENDIAN_ ? BYTE_SWAP_U64(*(uint64_t *)(&x)) : (x))
*/
/** to ignore no used warning */
//namespace bcus { static inline void ignore_this_function() { if(endian_test.mylong) {} }}


#if defined(_MSC_VER)
#include <intrin.h>
  //#define BCUS_CAS(ptr, old_value, new_value) (old_value == InterlockedCompareExchange(ptr, new_value, old_value))
  inline bool BCUS_CAS(unsigned char volatile *ptr, unsigned char old_value, unsigned char new_value) {
      char old_value_t = (char)old_value;
      return old_value_t == _InterlockedCompareExchange8((char volatile *)ptr, (char)new_value, (char)old_value);
  }
  inline bool BCUS_CAS(unsigned short volatile *ptr, unsigned short old_value, unsigned short new_value) {
      short old_value_t = (short)old_value;
      return old_value_t == _InterlockedCompareExchange16((short volatile *)ptr, (short)new_value, (short)old_value);
  }
  inline bool BCUS_CAS(unsigned int volatile *ptr, unsigned int old_value, unsigned int new_value) {
      long old_value_t = (long)old_value;
      return old_value_t == _InterlockedCompareExchange((long volatile *)ptr, (long)new_value, (long)old_value);
  }
  inline bool BCUS_CAS(unsigned long long volatile *ptr, unsigned long long old_value, unsigned long long new_value) {
      __int64 old_value_t = (__int64)old_value;
      return old_value_t == _InterlockedCompareExchange64((__int64 volatile *)ptr, (__int64)new_value, (__int64)old_value);
  }
#elif defined(__GNUC__)
  #define BCUS_CAS(ptr, old_value, new_value)     \
     __sync_bool_compare_and_swap(ptr, old_value, new_value)
#endif


#define BCUS_CPU_TICK ( )                                                 \
     do {                                                                  \
        unsigned long long tick = 0;                                       \
        unsigned long lo, hi;                                              \
        #if defined(__i386__)                                              \
        __asm__ __volatile__ (".byte 0x0f, 0x31" : "=a" (lo), "=d" (hi));  \
        tick = (unsigned long long) hi << 32 | lo;                         \
        #elif defined(__x86_64__)                                          \
        __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));             \
        tick = (unsigned long long) hi << 32 | lo;                         \
        #endif                                                             \
        tick;                                                              \
     } while(0)


static inline unsigned long long BCUS_CLOCK_TIME( ) {
    unsigned long long tick = 0;
  #if defined(_MSC_VER)
    FILETIME tm;
    ULONGLONG t;
    GetSystemTimeAsFileTime(&tm);
    tick = ((ULONGLONG)tm.dwHighDateTime<<32) | (ULONGLONG)tm.dwLowDateTime;
  #elif defined (LINUX)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    tick = ts.tv_sec << 32 | ts.tv_nsec;
  #elif defined(MAC)
    struct timeval ts;
    gettimeofday(&ts, NULL);
    tick = ts.tv_sec << 32 | (ts.tv_usec * 1000);
  #endif
    return tick;
}

#define _MIN(a, b) (a > b ? b : a)

#endif

