/**********************************************************************
 * Copyright (c) 2013, 2014 Pieter Wuille                             *
 * Distributed under the GPL3 software license, see the accompanying   *
 * file COPYING or http://www.gnu.org/licenses/gpl.html.*
 **********************************************************************/

#ifndef _SECP256K1_TESTRAND_H_
#define _SECP256K1_TESTRAND_H_

#if defined HAVE_CONFIG_H
#include "libsecp256k1-config.h"
#endif

/** Seed the pseudorandom number generator. */
SECP256K1_INLINE static void secp256k1_rand_seed(uint64_t v);

/** Generate a pseudorandom 32-bit number. */
static uint32_t secp256k1_rand32(void);

/** Generate a pseudorandom 32-byte array. */
static void secp256k1_rand256(unsigned char *b32);

/** Generate a pseudorandom 32-byte array with long sequences of zero and one bits. */
static void secp256k1_rand256_test(unsigned char *b32);

#endif
