#include <gtest/gtest.h>

#include "loghelper.h"
#include "stream_acceptor.h"
#include "stream_connector.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

static bcus::tcp_socket_ptr create_socket(boost::asio::io_service *io_service) {
    return bcus::tcp_socket_ptr(new bcus::tcp_socket(*io_service));
}

static void connect_callback(bcus::tcp_socket_ptr &socket,
                                  const bcus::endpoint &ep,
                                  const bcus::error_code &code,
                                  void *context)
{
    XLOG(XLOG_DEBUG, "%s, code[%d], fd[%d], ep[%s], context[%p]\n", __FUNCTION__,
        code, socket->native_handle(), ep.to_string().c_str(), context);
}

static void accept_callback(bcus::tcp_socket_ptr &socket)
{
    XLOG(XLOG_DEBUG, "%s, fd[%d]\n", __FUNCTION__, socket->native_handle());
}

TEST(net_acceptor, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    boost::asio::io_service io_service;

    bcus::stream_acceptor  accp(io_service);
    accp.set_create_socket_func(boost::bind(create_socket, &io_service));
    accp.set_callback(accept_callback);
    EXPECT_EQ(0, accp.start(bcus::endpoint("127.0.0.1", 10010)));

    uint64_t ctx1 = 0X1111;
    bcus::stream_connector conn1(io_service);
    conn1.set_create_socket_func(boost::bind(create_socket, &io_service));
    conn1.set_callback(connect_callback);
    conn1.connect("127.0.0.1", 10010, 3, (void *)ctx1);

    uint64_t ctx2 = 0X2222;
    bcus::stream_connector conn2(io_service);
    conn2.set_create_socket_func(boost::bind(create_socket, &io_service));
    conn2.set_callback(connect_callback);
    conn2.connect("127.0.0.1", 10010, 3, (void *)ctx2);


    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    thread.join();
}

