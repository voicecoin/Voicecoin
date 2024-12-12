#ifndef _WEB_COLLECTOR_CIPHER_H_
#define _WEB_COLLECTOR_CIPHER_H_

#include <string>
namespace bcus {

class cipher
{
public:
    static void sha256(const uint8_t *in, int len, uint8_t out[32]);
    static void ripemd160(const uint8_t *in, int len, uint8_t out[20]);

    static void bin_to_hex(const uint8_t *in, int inlen, char *out);
    static int  base64_encode(const uint8_t *in, int in_len, char *out) ;
    static int  base64_decode(const uint8_t *in, int in_len, char *out) ;
};

}

#endif
