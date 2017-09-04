#pragma once

#define BEGIN(a)            ((char*)&(a))
#define END(a)              ((char*)&((&(a))[1]))
#define UBEGIN(a)           ((unsigned char*)&(a))
#define UEND(a)             ((unsigned char*)&((&(a))[1]))
#define ARRAYLEN(array)     (sizeof(array)/sizeof((array)[0]))

#define BCUS_MAX(a,b) (((a) > (b)) ? (a) : (b))
#define BCUS_MIN(a,b) (((a) < (b)) ? (a) : (b))
