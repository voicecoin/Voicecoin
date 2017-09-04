#ifndef BCUS_NET_STREAM_CONNECTOR_H
#define BCUS_NET_STREAM_CONNECTOR_H

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include "endpoint.h"
#include "error_code.h"
#include "io_timer.h"
#include "tcp_channel.h"
#include <boost/function.hpp>
#include <set>

namespace bcus {

class stream_connector {
    typedef boost::function<tcp_socket_ptr()> create_socket_func;
    typedef boost::function<void (tcp_socket_ptr &,
                                  const bcus::endpoint &,
                                  const bcus::error_code &,
                                  void *context)> connect_callback_func;
public:
    stream_connector(boost::asio::io_service &io_service);
    ~stream_connector();

    template < typename FUNC>
    void set_create_socket_func(FUNC f) {
        create_socket_func_ = f;
    }
    template < typename FUNC>
    void set_callback(FUNC f) {
        callback_ = f;
    }
    int connect(const char *host, uint16_t port, int timeout_seconds = 3, void *context = NULL);

private:
    struct node {
        tcp_socket_ptr s;
        boost::asio::ip::tcp::resolver rs;
        std::string host;
        uint16_t port;
        void *context;
        io_timer timer;

        handle_alloc rs_alloc;
        handle_alloc conn_alloc;

        node(boost::asio::io_service &io_service) : rs(io_service) {}
    };

private:
    void handle_resolve(node *nd, const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void handle_connect(node *nd, const bcus::endpoint &ep,
        const boost::system::error_code& err,
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void handle_connect_timeout(node *nd);
    void connect_result(node *nd, const bcus::endpoint &ep, int code);
    void delete_node(node *nd);

private:
    boost::asio::io_service &io_service_;

    typedef std::set<node *> NODE_SET;
    NODE_SET nodes_;

    create_socket_func create_socket_func_;
    connect_callback_func callback_;
};

}

#endif
