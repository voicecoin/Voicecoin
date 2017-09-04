#include <gtest/gtest.h>

#include "loghelper.h"
#include "cipher.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>


TEST(crypt, sha256) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    const char *p = "test";
    uint8_t out[64] = {0};
    bcus::cipher::sha256((const uint8_t *)p, strlen(p), out);

    char hex[72] = {0};
    bcus::cipher::bin_to_hex((const uint8_t *)out, 32, hex);

    EXPECT_STREQ("9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08", hex);


    p = "hello";
    bcus::cipher::sha256((const uint8_t *)p, strlen(p), out);
    bcus::cipher::bin_to_hex((const uint8_t *)out, 32, hex);
    EXPECT_STREQ("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824", hex);

    bcus::cipher::sha256((const uint8_t *)out, 32, out);
    bcus::cipher::bin_to_hex((const uint8_t *)out, 32, hex);
    EXPECT_STREQ("9595c9df90075148eb06860365df33584b75bff782a510c6cd4883a419833d50", hex);

}
TEST(crypt, ripemd160) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    const char *p = "123456";
    uint8_t out[64] = {0};
    bcus::cipher::ripemd160((const uint8_t *)p, strlen(p), out);

    char hex[72] = {0};
    bcus::cipher::bin_to_hex((const uint8_t *)out, 20, hex);

    EXPECT_STREQ("d8913df37b24c97f28f840114d05bd110dbb2e44", hex);


    p = "hello";
    bcus::cipher::sha256((const uint8_t *)p, strlen(p), out);
    bcus::cipher::bin_to_hex((const uint8_t *)out, 32, hex);
    EXPECT_STREQ("2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824", hex);

    bcus::cipher::ripemd160((const uint8_t *)out, 32, out);
    bcus::cipher::bin_to_hex((const uint8_t *)out, 20, hex);
    EXPECT_STREQ("b6a9c8c230722b7c748331a8b450f05566dc7d0f", hex);

}


