#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "cipher.h"
#include "sha256.h"
#include "ripemd160.h"
#include "base64.h"

namespace bcus {

void cipher::sha256(const uint8_t *in, int len, uint8_t out[32])
{
    CSHA256 cc;
    cc.Write(in, len);
    cc.Finalize(out);
}
void cipher::ripemd160(const uint8_t *in, int len, uint8_t out[20])
{
    CRIPEMD160 cc;
    cc.Write(in, len);
    cc.Finalize(out);
}

void cipher::bin_to_hex(const uint8_t *in, int inlen, char *out)
{
    for (int i = 0; i < inlen; ++i) {
        sprintf((char *)(out + i * 2), "%02x", in[i]);
    }
}
int  cipher::base64_encode(const uint8_t *in, int in_len, char *out)
{
    int out_len = ::base64_encode(in, (uint8_t *)out, in_len, 1);
    *(out + out_len) = 0;
    return out_len;
}
int  cipher::base64_decode(const uint8_t *in, int in_len, char *out)
{
    int out_len = ::base64_decode(in, (uint8_t *)out, in_len);
    *(out + out_len) = 0;
    return out_len;
}

}
