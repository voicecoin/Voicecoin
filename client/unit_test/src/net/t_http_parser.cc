#include <gtest/gtest.h>

#include "loghelper.h"
#include "http_parser.h"


TEST(net_http_parser, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    bcus::http_encoder en;
    en.init("http://127.0.0.1:8008", "GET");
    en.head("Connection", "Keep-alive");
    en.encode();
    XLOG(XLOG_DEBUG, "%s\n", en.get_buf(), en.get_len());


    en.init("http://127.0.0.1:8008/tt", "GET");
    en.head("Connection", "Keep-alive");
    en.body("hello");
    en.encode();
    XLOG(XLOG_DEBUG, "%s\n", en.get_buf(), en.get_len());

    bcus::http_decoder d;
    EXPECT_EQ(en.get_len(), d.decode(en.get_buf(), en.get_len()));
    EXPECT_TRUE(d.is_request());
    EXPECT_STREQ("GET", d.http_method());
    EXPECT_STREQ("/tt", d.url_path());
    EXPECT_STREQ("hello", d.body().c_str());

    char sz[1024] = {0};
    int len = en.get_len();
    memcpy(sz, en.get_buf(), len);
    memcpy(sz + en.get_len(), en.get_buf(), len);
    len += len;

    int len1 = d.decode(sz, len);
    EXPECT_EQ(en.get_len(), len1);
    EXPECT_TRUE(d.is_request());
    EXPECT_STREQ("GET", d.http_method());
    EXPECT_STREQ("/tt", d.url_path());
    EXPECT_STREQ("hello", d.body().str().c_str());

    len1 = d.decode(sz + len1, len - len1);
    EXPECT_EQ(en.get_len(), len1);
    EXPECT_TRUE(d.is_request());
    EXPECT_STREQ("GET", d.http_method());
    EXPECT_STREQ("/tt", d.url_path());
    EXPECT_STREQ("hello", d.body().str().c_str());
}

