#include <gtest/gtest.h>

#include "loghelper.h"
#include "bitcoin_message.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>


TEST(net_bitcoin_message, c1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    bcus::bitcoin_encoder en;
    bcus::bitcoin_message_factory::create_version_msg(&en, bcus::endpoint("1.1.1.1", 1001), 0X1111, 0X2222);
    XLOG(XLOG_DEBUG, "\n%s\n", binary_dump_string(en.get_buf(), en.get_len()).c_str());

    bcus::bitcoin_decoder d;
    d.decode(en.get_buf(), en.get_len());
    XLOG(XLOG_DEBUG, "%s\n", d.to_string().c_str());

    bcus::slice body = d.body();
    bcus::version_msg msg;
    msg.init_from_stream(body.c_str(), body.length());
    msg.dump();
}

