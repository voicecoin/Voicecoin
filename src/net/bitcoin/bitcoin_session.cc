#include "bitcoin_session.h"
#include "loghelper.h"
#include "channel_factory.h"

namespace bcus {

bitcoin_session::bitcoin_session( boost::asio::io_service &io_service, tcp_socket_ptr &socket) : io_service_(io_service)
{
    static const int STATUS_TIMEOUT_CHECK_INTERVAL = 30;  //30s

    XLOG(XLOG_DEBUG, "bitcoin_session::%s, fd[%d]\n", __FUNCTION__, socket->native_handle());
    channel_type *channel = new channel_type(
                                    io_service_,
                                    this,
                                    socket,
                                    STATUS_TIMEOUT_CHECK_INTERVAL);

    channel_.reset(channel);
    channel_->set_owner(this);
    channel_->async_read();
}
bitcoin_session::~bitcoin_session() {
    XLOG(XLOG_DEBUG, "bitcoin_session::%s\n", __FUNCTION__);
}
void bitcoin_session::on_read(bcus::bitcoin_decoder *d, const bcus::endpoint &ep)
{
    //XLOG(XLOG_DEBUG, "bitcoin_session::%s, %s\n", __FUNCTION__, d->to_string().c_str());
    read_callback_(this, d, ep);
}
int  bitcoin_session::send(const void *buf, int len)
{
    channel_->async_write(buf, len);
    return 0;
}
void bitcoin_session::on_peer_close() {
    close_callback_(this, channel_->remote_endpoint());
}
void bitcoin_session::close()
{
    channel_->set_owner(NULL);
    channel_->close();
}


}

