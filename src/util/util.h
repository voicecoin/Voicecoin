#pragma once

#define BEGIN(a)            ((char*)&(a))
#define END(a)              ((char*)&((&(a))[1]))
#define UBEGIN(a)           ((unsigned char*)&(a))
#define UEND(a)             ((unsigned char*)&((&(a))[1]))
#define ARRAYLEN(array)     (sizeof(array)/sizeof((array)[0]))

#ifndef max
#  define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#  define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
