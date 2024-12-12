// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the GPL3 software license, see the accompanying
// file COPYING or http://www.gnu.org/licenses/gpl.html.

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include <cstddef>

#if defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

// Prior to GLIBC_2.14, memcpy was aliased to memmove.
extern "C" void* memmove(void* a, const void* b, size_t c);
extern "C" void* memcpy(void* a, const void* b, size_t c)
{
    return memmove(a, b, c);
}

extern "C" void __chk_fail(void) __attribute__((__noreturn__));
extern "C" FDELT_TYPE __fdelt_warn(FDELT_TYPE a)
{
    if (a >= FD_SETSIZE)
        __chk_fail();
    return a / __NFDBITS;
}
extern "C" FDELT_TYPE __fdelt_chk(FDELT_TYPE) __attribute__((weak, alias("__fdelt_warn")));
