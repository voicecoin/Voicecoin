#pragma once

#include "uint256.h"
#include <vector>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

class hash_helper
{
public:
    template<typename T1>
    static uint256 hash(const T1 pbegin, const T1 pend)
    {
        // important things are to be repeated for 3 times
        uint256 hash1;
        SHA256((unsigned char*)&pbegin[0], (pend - pbegin) * sizeof(pbegin[0]), (unsigned char*)&hash1);
        uint256 hash2;
        SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
        uint256 hash3;
        SHA256((unsigned char*)&hash2, sizeof(hash2), (unsigned char*)&hash3);
        return hash3;
    }

    template<typename T1, typename T2>
    static uint256 hash(const T1 p1begin, const T1 p1end,
        const T2 p2begin, const T2 p2end)
    {
        uint256 hash1;
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, (unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]));
        SHA256_Update(&ctx, (unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]));
        SHA256_Final((unsigned char*)&hash1, &ctx);
        uint256 hash2;
        SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
        uint256 hash3;
        SHA256((unsigned char*)&hash2, sizeof(hash2), (unsigned char*)&hash3);
        return hash3;
    }

    template<typename T1, typename T2, typename T3>
    static uint256 hash(const T1 p1begin, const T1 p1end,
        const T2 p2begin, const T2 p2end,
        const T3 p3begin, const T3 p3end)
    {
        uint256 hash1;
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, (unsigned char*)&p1begin[0], (p1end - p1begin) * sizeof(p1begin[0]));
        SHA256_Update(&ctx, (unsigned char*)&p2begin[0], (p2end - p2begin) * sizeof(p2begin[0]));
        SHA256_Update(&ctx, (unsigned char*)&p3begin[0], (p3end - p3begin) * sizeof(p3begin[0]));
        SHA256_Final((unsigned char*)&hash1, &ctx);
        uint256 hash2;
        SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
        uint256 hash3;
        SHA256((unsigned char*)&hash2, sizeof(hash2), (unsigned char*)&hash3);
        return hash3;
    }

    static uint160 hash160(const std::vector<unsigned char>& vch)
    {
        uint256 hash1;
        SHA256(&vch[0], vch.size(), (unsigned char*)&hash1);
        uint256 hash2;
        SHA256((unsigned char*)&hash1, sizeof(hash1), (unsigned char*)&hash2);
        uint160 hash3;
        RIPEMD160((unsigned char*)&hash2, sizeof(hash2), (unsigned char*)&hash3);
        return hash3;
    }
};
