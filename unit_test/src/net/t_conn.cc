#include <gtest/gtest.h>

#include "loghelper.h"
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

TEST(net_connector, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    boost::asio::io_service io_service;
    bcus::stream_connector conn(io_service);

    uint64_t ctx = 0X1111;
    conn.set_create_socket_func(boost::bind(create_socket, &io_service));
    conn.set_callback(connect_callback);
    conn.connect("www.baidu.com", 80, 3, (void *)ctx);

    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    thread.join();
}

