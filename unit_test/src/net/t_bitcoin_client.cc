#include <gtest/gtest.h>

#include "loghelper.h"
#include "bitcoin_client.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

static void read_callback(bcus::bitcoin_decoder *d)
{
    XLOG(XLOG_DEBUG, "%s\n%s\n", __FUNCTION__, d->to_string().c_str());
}


TEST(net_bitcoin_client, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    boost::asio::io_service io_service;
    bcus::bitcoin_client client(io_service);
    client.set_read_callback(read_callback);

    client.register_addr("seed.bitcoin.sipa.be", 8333);

    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    thread.join();
}

