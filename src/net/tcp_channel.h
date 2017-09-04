#ifndef BCUS_NET_TCP_CHANNEL_H
#define BCUS_NET_TCP_CHANNEL_H

#include <boost/asio.hpp>
#include "channel.h"

namespace bcus {

class tcp_socket : public boost::asio::ip::tcp::socket {
  public:
    tcp_socket(boost::asio::io_service &io_service) : boost::asio::ip::tcp::socket(io_service) { }
    virtual ~tcp_socket() {
        XLOG(XLOG_DEBUG, "tcp_socket::%s, fd[%d]\n", __FUNCTION__, native_handle());
    }
    /*void assign(const bcus::socket_handle &fd) {
        // EEXIST will return in epoll_add which is called by reactive_socket_service_base::do_assign
        // we make it correct
        boost::system::error_code ec;
        boost::asio::ip::tcp::socket::assign(boost::asio::ip::tcp::v4(), fd, ec);
        if (ec) {
            XLOG(XLOG_DEBUG, "tcp_socket::%s, e[%s]\n", __FUNCTION__, ec.message().c_str());
            get_implementation().socket_ = fd;
            get_implementation().state_ =   boost::asio::detail::socket_ops::stream_oriented |
                                            boost::asio::detail::socket_ops::possible_dup;
        }
    }*/
};
typedef boost::shared_ptr<tcp_socket> tcp_socket_ptr;


template<typename owner_type, typename decoder>
class tcp_channel : public channel_tpl<boost::asio::ip::tcp::socket, owner_type, decoder> {
    typedef channel_tpl<boost::asio::ip::tcp::socket, owner_type, decoder> base_type;
public:
    tcp_channel(    boost::asio::io_service &io_service,
                    owner_type *owner,
                    tcp_socket_ptr &socket,
                    uint32_t status_check_sec);
    virtual ~tcp_channel();

private:
    tcp_socket_ptr socket_;
};

template<typename owner_type, typename decoder>
tcp_channel<owner_type, decoder>::tcp_channel(  boost::asio::io_service &io_service,
                                                owner_type *owner,
                                                tcp_socket_ptr &socket,
                                                uint32_t status_check_sec) :
    base_type(io_service, status_check_sec)
{
    XLOG(XLOG_DEBUG, "tcp_channel::%s\n", __FUNCTION__);
    socket_ = socket;
    base_type::set_owner(owner);
    base_type::set_socket(socket_.get());
}

template<typename owner_type, typename decoder>
tcp_channel<owner_type, decoder>::~tcp_channel()
{
    XLOG(XLOG_DEBUG, "tcp_channel::%s, fd[%u]\n", __FUNCTION__, socket_->native_handle());
}

}
#endif

