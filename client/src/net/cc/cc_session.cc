#include "cc_session.h"
#include "loghelper.h"
#include "channel_factory.h"
#include "cc_command.h"

namespace bcus {

cc_session::cc_session( boost::asio::io_service &io_service, tcp_socket_ptr &socket) : io_service_(io_service)
{
    static const int STATUS_TIMEOUT_CHECK_INTERVAL = 30;  //30s

    XLOG(XLOG_DEBUG, "cc_session::%s, fd[%d]\n", __FUNCTION__, socket->native_handle());
    channel_type *channel = new channel_type(
                                    io_service_,
                                    this,
                                    socket,
                                    STATUS_TIMEOUT_CHECK_INTERVAL);

    channel_.reset(channel);
    channel_->set_owner(this);
    channel_->async_read();

    ping_timer_.init(&io_service_, 10000, boost::bind(&cc_session::do_ping, this), bcus::io_timer::TIMER_CIRCLE);
    ping_timer_.start();
}
cc_session::~cc_session() {
    XLOG(XLOG_DEBUG, "cc_session::%s\n", __FUNCTION__);
    ping_timer_.stop();
}
void cc_session::on_read(bcus::cc_decoder *d, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "cc_session::%s, %s\n", __FUNCTION__, d->to_string().c_str());
    switch (d->get_command()) {
      case CC_PING: {
        do_pong();
        break;
      }
      default: {
        read_callback_(this, d, ep);
        break;
      }
    }
}
int  cc_session::send(const void *buf, int len)
{
    channel_->async_write(buf, len);
    return 0;
}
void cc_session::on_peer_close() {
    close_callback_(this, channel_->remote_endpoint());
}
void cc_session::close()
{
    channel_->set_owner(NULL);
    channel_->close();
}
void cc_session::do_ping()
{
    XLOG(XLOG_DEBUG, "cc_session::%s\n", __FUNCTION__);
    en_.init(MAINNET, CC_PING);
    en_.encode();
    send(en_.get_buf(), en_.get_len());
}
void cc_session::do_pong()
{
    en_.init(MAINNET, CC_PONG);
    en_.encode();
    send(en_.get_buf(), en_.get_len());
}


}

