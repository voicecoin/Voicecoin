#include <gtest/gtest.h>

#include "loghelper.h"
#include "http_client.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

static void read_callback(bcus::http_decoder *d, const bcus::endpoint &ep, void *context)
{
    XLOG(XLOG_DEBUG, "%s, context[%p], \n%s\n", __FUNCTION__,
        context, d->to_string().c_str());
}

static void thread_run(boost::asio::io_service *io_service) {
    while(1) {
        int ret = io_service->run_one();
        XLOG(XLOG_DEBUG, "%s, ret[%d]\n", __FUNCTION__, ret);
    }
}

TEST(net_http_client, t1) {
    XLOG(XLOG_DEBUG, "%s, boost::asio::error::already_open[%d]\n", typeid(this).name(), boost::asio::error::already_open);

    boost::asio::io_service io_service;
    bcus::http_client client(io_service);
    client.set_read_callback(read_callback);

    uint64_t ctx = 0X1111;
    //client.request("http://127.0.0.1:8008", (void *)ctx);
    client.request("http://127.0.0.1:8008/test.html", (void *)ctx);
    //client.request("http://172.31.197.3:8008", (void *)ctx);

    boost::thread thread(boost::bind(thread_run, &io_service));
    thread.join();
}

