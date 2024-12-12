#ifndef BCUS_NET_CHANNEL_H
#define BCUS_NET_CHANNEL_H

#include "noncopyable.h"
#include "buffer.h"
#include "ring_buffer.h"
#include "loghelper.h"
#include "endpoint.h"
#include "io_timer.h"
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace bcus {

class channel {
public:
    virtual ~channel() {}
    virtual void async_read() = 0;
    virtual int  async_write(const void *buf, int len) = 0;
    virtual void close() = 0;
    virtual bool is_connected() = 0;
    virtual void set_owner(void *owner) = 0;
    virtual const bcus::endpoint &remote_endpoint() = 0;
    //virtual void target_endpoint(const bcus::endpoint &) = 0;
    //virtual const bcus::endpoint &target_endpoint() const = 0;
};

template<typename socket_type, typename owner_type, typename decoder>
class channel_tpl : public channel,
                    public boost::enable_shared_from_this<channel_tpl<socket_type, owner_type, decoder> > ,
                    public bcus::noncopyable
{
    typedef boost::enable_shared_from_this<channel_tpl<socket_type, owner_type, decoder> > shared_type;
protected:
    typedef enum {E_CREATED = 0, E_CONNECTED, E_TIMEOUT, E_PEER_CLOSE, E_CLOSED } channel_status_type;
public:
    typedef channel_tpl<socket_type, owner_type, decoder> this_type;

    channel_tpl(boost::asio::io_service &io_service, uint32_t status_check_sec);
    virtual ~channel_tpl();
    virtual void async_read();
    virtual int  async_write(const void *buf, int len);
    virtual void close();
    virtual bool is_connected() {
        return status_ == E_CONNECTED || status_ == E_TIMEOUT;
    }
    virtual void set_owner(void *owner) {
        owner_ = (owner_type *)owner;
    }
    virtual const bcus::endpoint &remote_endpoint() {
        if (remote_endpoint_.empty()) {
            boost::system::error_code ec;
            remote_endpoint_.ip(socket_->remote_endpoint(ec).address().to_string().c_str());
            remote_endpoint_.port(socket_->remote_endpoint(ec).port());
        }
        return remote_endpoint_;
    }/*
    virtual void target_endpoint(const bcus::endpoint &ep) {
        return socket_->target_endpoint(ep);
    }
    virtual const bcus::endpoint &target_endpoint() const {
        return socket_->target_endpoint();
    }*/
protected:
    void set_socket(socket_type *s) { socket_ = s; }

private:
    void async_read_inner();
    void handle_read(const boost::system::error_code& err, std::size_t size);
    void async_write_inner();
    void handle_write(const boost::system::error_code& err, std::size_t size);
    void do_statuc_check();

protected:
    channel_status_type status_;
    boost::asio::io_service &io_service_;
    socket_type *socket_;
    owner_type *owner_;
    decoder *decoder_;

    bcus::buffer buffer_;

    bcus::io_timer  timer_status_check_;
    bool is_dealing_read_;

    handle_alloc read_alloc_;
    handle_alloc write_alloc_;

    bcus::endpoint remote_endpoint_;

    bcus::ring_buffer write_cache_;
};


template<typename socket_type, typename owner_type, typename decoder>
channel_tpl<socket_type, owner_type, decoder>::channel_tpl(boost::asio::io_service &io_service, uint32_t status_check_sec) : io_service_(io_service), owner_(NULL)
{
    decoder_ = new decoder();
    status_ = E_CREATED;

    if (status_check_sec > 0) {
        timer_status_check_.init(&io_service_, status_check_sec * 1000,
            boost::bind(&this_type::do_statuc_check, this), bcus::io_timer::TIMER_CIRCLE);
        timer_status_check_.start();
    }
    is_dealing_read_ = false;
}

template<typename socket_type, typename owner_type, typename decoder>
channel_tpl<socket_type, owner_type, decoder>::~channel_tpl()
{
    XLOG(XLOG_DEBUG, "channel_tpl::%s\n", __FUNCTION__);
    if (decoder_)
    {
        delete decoder_;
        decoder_ = NULL;
    }
    timer_status_check_.stop();
    close();
}

template<typename socket_type, typename owner_type, typename decoder>
void channel_tpl<socket_type, owner_type, decoder>::async_read()
{
    io_service_.post(boost::bind(&channel_tpl::async_read_inner, shared_type::shared_from_this()));
}

template<typename socket_type, typename owner_type, typename decoder>
void channel_tpl<socket_type, owner_type, decoder>::async_read_inner()
{
    status_ = E_CONNECTED;
    if (!is_dealing_read_) {
        //buffer_.reset_loc(0);
        socket_->async_read_some( boost::asio::buffer(buffer_.top(), buffer_.capacity() - 1),
                make_alloc_handler(read_alloc_,
                    boost::bind(&this_type::handle_read, shared_type::shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
    }
}

template<typename socket_type, typename owner_type, typename decoder>
void channel_tpl<socket_type, owner_type, decoder>::handle_read(const boost::system::error_code& ec, std::size_t bytes_read)
{
    bcus::endpoint ep = remote_endpoint();
    if (ec || bytes_read == 0)
    {
        buffer_.reset_loc(0);
        XLOG(XLOG_WARNING, "channel_tpl::%s, addr[%s], err[%s]\n", __FUNCTION__, ep.to_string().c_str(), ec.message().c_str());
        close();
        return;
    }
    status_ = E_CONNECTED;
    buffer_.inc_loc(bytes_read);
    *(buffer_.top()) = 0;

    static const unsigned int MAX_PACKET_SIZE = 16 * 1024 * 1024;  //16M

    if (buffer_.len() > MAX_PACKET_SIZE)
    {
        XLOG(XLOG_WARNING, "channel_tpl::%s, too big packet, addr[%s]\n", __FUNCTION__, ep.to_string().c_str());
        close();
        return;
    }
    char *p = buffer_.base();
    while (p < buffer_.top())
    {
        is_dealing_read_ = true;
        int len = buffer_.top() - p;
        int ret = decoder_->decode(p, len);

        if (ret < 0 || ret > len)
        {
            //XLOG(XLOG_WARNING,"channel_tpl::%s, addr[%s:%u], illegal packet, len[%d], buf::\n%s\n", __FUNCTION__, ip, port, len, binary_dump_string(p, len).c_str());
            XLOG(XLOG_WARNING,"channel_tpl::%s, addr[%s], illegal packet, len[%d], ret[%d]\n", __FUNCTION__, ep.to_string().c_str(), len, ret);
            close();
            return;
        }
        else if (ret == 0) //incomplete packet
        {
            //XLOG(XLOG_DEBUG,"channel_tpl::%s, addr[%s:%u], incomplete packet, waiting for read\n", __FUNCTION__, ip, port);
            break;
        }
        else // 0 < ret <= len
        {
            if(owner_ != NULL)
            {
                //XLOG(XLOG_DEBUG, "channel_tpl::%s, [%s:%u] \n%s\n", __FUNCTION__,
                //    remote_ip(), remote_port(), binary_dump_string(p, len).c_str());
                owner_->on_read(decoder_, ep);
            }
            else
            {
                XLOG(XLOG_WARNING,"channel_tpl::%s::%d, session is NULL, addr[%s], bytes_read[%d], buffer_.len[%d]\n",
                    __FUNCTION__, __LINE__, ep.to_string().c_str(), bytes_read, buffer_.len());
                close();
                return;
            }
        }
        p += ret;
    }

    int left = 0;
    if (p < buffer_.top())
    {
        //XLOG(XLOG_TRACE,"channel_tpl::%s::%d, read packet over, memmove the packet, len[%d]\n",__FUNCTION__,__LINE__, buffer_.top() - p);
        left = buffer_.top() - p;
        memmove(buffer_.base(), p, left);
    }
    buffer_.reset_loc(left);
    if (buffer_.capacity() <= 4096)
    {
        //XLOG(XLOG_TRACE,"channel_tpl::%s, increase 4096 bytes\n",__FUNCTION__);
        buffer_.add_capacity();
    }

    async_read_inner();
}

template<typename socket_type, typename owner_type, typename decoder>
int  channel_tpl<socket_type, owner_type, decoder>::async_write(const void *buf, int len)
{
    bool to_send = write_cache_.empty();
    write_cache_.append((const char *)buf, len);
    if (to_send) {
        async_write_inner();
    }
    return 0;
}
template<typename socket_type, typename owner_type, typename decoder>
void  channel_tpl<socket_type, owner_type, decoder>::async_write_inner()
{
    if (write_cache_.empty()) {
        return;
    }
    static const std::size_t MAX_SEND_SIZE  = 1460;  //the max ttl is 1500
    int len = write_cache_.len() > MAX_SEND_SIZE ? MAX_SEND_SIZE : write_cache_.len();
    socket_->async_send(boost::asio::buffer(write_cache_.head(), len),
            make_alloc_handler(write_alloc_,
                boost::bind(&this_type::handle_write, shared_type::shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
}


template<typename socket_type, typename owner_type, typename decoder>
void channel_tpl<socket_type, owner_type, decoder>::handle_write(const boost::system::error_code& err, std::size_t size)
{
    if (err) {
        XLOG(XLOG_WARNING, "channel_tpl::%s, remote[%s] writebuf.size[%u], error[%s]\n",
            __FUNCTION__, remote_endpoint().to_string().c_str(), write_cache_.len(), err.message().c_str());
        close();
        return;
    }
    write_cache_.pop_front(size);
    async_write_inner();
}

template<typename socket_type, typename owner_type, typename decoder>
void channel_tpl<socket_type, owner_type, decoder>::close()
{
    XLOG(XLOG_DEBUG, "channel_tpl::%s, status_[%d], owner_[%p]\n", __FUNCTION__, status_, owner_);
    if (status_ != E_CLOSED)
    {
        status_ = E_CLOSED;
        //socket_->cancel();
        boost::system::error_code errcode;
        socket_->close(errcode);
    }
    if (owner_)
    {
        owner_type *p = owner_;
        owner_ = NULL;
        p->on_peer_close(); //may delete channel_tpl
    }
}

template<typename socket_type, typename owner_type, typename decoder>
void channel_tpl<socket_type, owner_type, decoder>::do_statuc_check()
{
    switch(status_) {
      case E_CONNECTED:
        status_ = E_TIMEOUT;
        break;
      case E_TIMEOUT:
        status_ = E_PEER_CLOSE;
        break;
      case E_PEER_CLOSE: {
        //const bcus::endpoint &ep = remote_endpoint().empty() ? target_endpoint() : remote_endpoint();
        const bcus::endpoint &ep = remote_endpoint().empty();
        XLOG(XLOG_WARNING, "channel_tpl::%s, addr[%s] timeout \n",
            __FUNCTION__, ep.to_string().c_str());
        timer_status_check_.stop();
        close();
        break;
      }
      default:
        break;
    }
}

}

#endif

