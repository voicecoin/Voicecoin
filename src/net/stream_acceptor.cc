#include "stream_acceptor.h"
#include "loghelper.h"
#include <stdexcept>
#include <boost/bind.hpp>

namespace bcus {

stream_acceptor::stream_acceptor(boost::asio::io_service &io_service) : io_service_(io_service), acceptor_(io_service_)
{
}
stream_acceptor::~stream_acceptor() {
}

int stream_acceptor::start(const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "stream_acceptor::%s, addr[%s]\n", __FUNCTION__, ep.to_string().c_str());
    boost::system::error_code ec;
    acceptor_.open(boost::asio::ip::tcp::v4(), ec);
    if(ec) {
        XLOG(XLOG_ERROR, "stream_acceptor::%s, [%s], open socket error[%s]\n", __FUNCTION__, ep.to_string().c_str(), ec.message().c_str());
        acceptor_.close(ec);
        return -1;
    }
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    acceptor_.set_option(boost::asio::ip::tcp::no_delay(true), ec);
    acceptor_.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(ep.ip()), ep.port()),ec);
    if(ec) {
        XLOG(XLOG_ERROR, "stream_acceptor::%s, [%s], bind socket error[%s]\n", __FUNCTION__, ep.to_string().c_str(), ec.message().c_str());
        acceptor_.close(ec);
        return -1;
    }
    acceptor_.listen(boost::asio::socket_base::max_connections, ec);
    if(ec) {
        XLOG(XLOG_ERROR, "stream_acceptor::%s, [%s], listen socket error[%s]\n", __FUNCTION__, ep.to_string().c_str(), ec.message().c_str());
        acceptor_.close(ec);
        return -3;
    }
    start_accept();
    return 0;
}

void stream_acceptor::start_accept() {
    XLOG(XLOG_DEBUG, "stream_acceptor::%s\n", __FUNCTION__);
    current_socket_ = create_socket_func_();
    acceptor_.async_accept(*(current_socket_.get()),
                make_alloc_handler(alloc_, boost::bind(&stream_acceptor::handle_accept, this,
                    boost::asio::placeholders::error)));
}

void stream_acceptor::handle_accept(const boost::system::error_code &err) {
    XLOG(XLOG_DEBUG, "stream_acceptor::%s\n", __FUNCTION__);
    if (!err) {
        callback_(current_socket_);
    } else {
        XLOG(XLOG_WARNING, "stream_acceptor::handle_accept, err[%s]\n", err.message().c_str());
    }
    start_accept();
}


}

