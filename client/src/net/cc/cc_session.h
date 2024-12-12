#ifndef BCUS_CC_SESSION_H
#define BCUS_CC_SESSION_H

#include <boost/asio/io_service.hpp>
#include "session.h"
#include "endpoint.h"
#include "tcp_channel.h"
#include "cc_parser.h"
#include "io_timer.h"
#include <boost/shared_ptr.hpp>
#include <deque>

namespace bcus {

class cc_session : public bcus::session {

    typedef boost::function<void (  bcus::cc_session *session,
                                    cc_decoder *d,
                                    const bcus::endpoint &)> READ_CALLBACK;

    typedef boost::function<void (  bcus::cc_session *session,
                                    const bcus::endpoint &ep) > CLOSE_CALLBACK;

    typedef cc_session this_type;

    typedef bcus::tcp_channel<bcus::cc_session, bcus::cc_decoder> channel_type;
    typedef boost::shared_ptr<channel_type> channel_ptr;

public:
    cc_session( boost::asio::io_service &io_service, tcp_socket_ptr &socket);

    virtual ~cc_session();

    template<typename Function>
    void set_read_callback(Function cb) {
        read_callback_ = cb;
    }
    template<typename Function>
    void set_close_callback(Function cb) {
        close_callback_ = cb;
    }

    /** called by channel */
    void on_read(bcus::cc_decoder *d, const bcus::endpoint &ep);
    void on_peer_close();

    void close();
    int  send(const void *buf, int len);

private:
    void do_ping();
    void do_pong();

private:
    boost::asio::io_service  &io_service_;
    READ_CALLBACK  read_callback_;
    CLOSE_CALLBACK close_callback_;

    channel_ptr channel_;

    bcus::io_timer ping_timer_;
    cc_encoder en_;
};


}

#endif
