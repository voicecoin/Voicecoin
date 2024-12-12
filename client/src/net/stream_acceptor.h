#ifndef BCUS_NET_STREAM_ACCEPTOR_H
#define BCUS_NET_STREAM_ACCEPTOR_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include "endpoint.h"
#include "handle_alloc.h"
#include "tcp_channel.h"

namespace bcus {

class stream_acceptor {
    typedef boost::function<tcp_socket_ptr ()> create_socket_func;
    typedef boost::function<void (tcp_socket_ptr &)> accept_callback_func;
public:
    stream_acceptor(boost::asio::io_service &io_service);
    ~stream_acceptor();

    template < typename FUNC>
    void set_create_socket_func(FUNC f) {
        create_socket_func_ = f;
    }
    template < typename FUNC>
    void set_callback(FUNC f) {
        callback_ = f;
    }

    int start(const bcus::endpoint &endpoint);

private:
    void start_accept();
    void handle_accept(const boost::system::error_code &err);

private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    handle_alloc alloc_;

    create_socket_func create_socket_func_;
    accept_callback_func callback_;

    tcp_socket_ptr current_socket_;
};

}

#endif
