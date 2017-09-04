#include "stream_connector.h"
#include "loghelper.h"
#include "string_helper.h"
#include <stdexcept>
#include <boost/bind.hpp>

namespace bcus {

stream_connector::stream_connector(boost::asio::io_service &io_service) : io_service_(io_service)
{
}
stream_connector::~stream_connector() {
    for(NODE_SET::iterator itr = nodes_.begin(); itr != nodes_.end(); ++itr) {
        (*itr)->s->cancel();
        (*itr)->rs.cancel();
        delete *itr;
    }
    nodes_.clear();
}


int stream_connector::connect(const char *host, uint16_t port, int timeout_seconds, void *context) {
    node *nd = new node(io_service_);
    nd->s = create_socket_func_();

    boost::asio::ip::tcp::resolver::query query(host, string_helper::format("%u", port));
    nd->rs.async_resolve(query,
            make_alloc_handler(nd->rs_alloc,
                boost::bind(&stream_connector::handle_resolve, this, nd,
                             boost::asio::placeholders::error,
                             boost::asio::placeholders::iterator)));

    nd->host = host;
    nd->port = port;
    nd->context = context;

    nd->timer.init(&io_service_, timeout_seconds * 1000,
        boost::bind(&stream_connector::handle_connect_timeout, this, nd),
        bcus::io_timer::TIMER_ONCE);
    nd->timer.start();

    nodes_.insert(nd);
    return 0;
}

void stream_connector::handle_resolve(stream_connector::node *nd, const boost::system::error_code& err,
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
    if (err) {
        XLOG(XLOG_WARNING,"stream_connector::%s, addr[%s:%s], error:%s\n", __FUNCTION__,
            nd->host.c_str(), nd->port, (err.message()).c_str());
        connect_result(nd, bcus::endpoint(), err.value());
        return;
    }

    boost::asio::ip::tcp::endpoint ep = *endpoint_iterator;
    XLOG(XLOG_DEBUG, "stream_connector::%s, addr[%s:%u], resolved addr[%s:%d]\n", __FUNCTION__,
        nd->host.c_str(), nd->port,
        ep.address().to_string().c_str(), ep.port());

    nd->s->async_connect(ep, make_alloc_handler(nd->conn_alloc,
                boost::bind(&stream_connector::handle_connect, this, nd,
                bcus::endpoint(ep.address().to_string().c_str(), ep.port()),
                boost::asio::placeholders::error, ++endpoint_iterator)));
}

void stream_connector::handle_connect(stream_connector::node *nd,
    const bcus::endpoint &ep,
    const boost::system::error_code& err,
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
    XLOG(XLOG_DEBUG, "stream_connector::%s, addr[%s:%u], ep[%s], err[%d]\n", __FUNCTION__,
        nd->host.c_str(), nd->port, ep.to_string().c_str(), err);
    if (!err) {
        connect_result(nd, ep, err.value());  //success
        return;
    }
    if (endpoint_iterator == boost::asio::ip::tcp::resolver::iterator()) {
        XLOG(XLOG_WARNING, "stream_connector::%s, addr[%s:%u], error:%s\n", __FUNCTION__,
            nd->host.c_str(), nd->port, (err.message()).c_str());
        connect_result(nd, ep, bcus::error::host_unreachable);
        return;
    }

    boost::system::error_code ignore_ec;
    nd->s->close(ignore_ec);
    boost::asio::ip::tcp::endpoint ep2 = *endpoint_iterator;
    nd->s->async_connect(ep2, make_alloc_handler(nd->conn_alloc,
                boost::bind(&stream_connector::handle_connect, this, nd,
                bcus::endpoint(ep2.address().to_string().c_str(), ep2.port()),
                boost::asio::placeholders::error, ++endpoint_iterator)));
}
void stream_connector::handle_connect_timeout(stream_connector::node *nd)
{
    XLOG(XLOG_DEBUG, "stream_connector::%s, addr[%s:%u]\n", __FUNCTION__, nd->host.c_str(), nd->port);

    boost::system::error_code ignore_ec;
    nd->s->cancel();
    nd->s->close(ignore_ec);
    nd->rs.cancel();
    connect_result(nd, bcus::endpoint(), bcus::error::timed_out);
}
void stream_connector::delete_node(stream_connector::node *nd) {
    delete nd;
}
void stream_connector::connect_result(stream_connector::node *nd, const bcus::endpoint &ep, int code) {
    XLOG(XLOG_DEBUG, "stream_connector::%s, addr[%s], code[%d]\n", __FUNCTION__, ep.to_string().c_str(), code);

    nd->timer.stop();
    nd->s->cancel();
    nd->rs.cancel();
    nodes_.erase(nd);
    callback_(nd->s, ep, code, nd->context);
    //delete nd;
    /** we must delete node in another loop circle,
    becase the next loop circle will call operation which is in handle_alloc*/
    io_service_.post(boost::bind(&stream_connector::delete_node, this, nd));
}

}

