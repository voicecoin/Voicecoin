#include <gtest/gtest.h>

#include "loghelper.h"
#include "http_client.h"
#include "http_server.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

static void server_read_callback(  bcus::http_server *server,
                            uint32_t containerid,
                            uint32_t sessionid,
                            bcus::http_decoder *d,
                            const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "%s, containerid[%u], sessionid[%u], remote[%s]\n%s\n", __FUNCTION__,
        containerid, sessionid, ep.to_string().c_str(), d->to_string().c_str());

    bcus::http_encoder en;
    en.init(200);
    en.body("this is response message");
    server->send_response( containerid,  sessionid,  d->seqno(), en.get_buf(), en.get_len());
}

static void client_read_callback(bcus::http_decoder *d, const bcus::endpoint &ep, void *context)
{
    XLOG(XLOG_DEBUG, "%s, context[%p], \n%s\n", __FUNCTION__, context, d->to_string().c_str());
}

static void thread_run(boost::asio::io_service *io_service) {
    while(1) {
        int ret = io_service->run_one();
        XLOG(XLOG_DEBUG, "%s, ret[%d]\n", __FUNCTION__, ret);
    }
}

TEST(net_http_server, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    boost::asio::io_service io_service;
    bcus::http_server server(io_service);
    server.set_read_callback(boost::bind(server_read_callback, &server, _1, _2, _3, _4));
    server.register_addr(bcus::endpoint("127.0.0.1", 10010));


    bcus::http_client client(io_service);
    client.set_read_callback(client_read_callback);

    uint64_t ctx = 0X1111;
    client.request("http://127.0.0.1:10010/tt", (void *)ctx);

    boost::thread thread(boost::bind(thread_run, &io_service));
    thread.join();
}

