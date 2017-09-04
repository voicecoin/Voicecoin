#ifndef BCUS_BITCOIN_SESSION_H
#define BCUS_BITCOIN_SESSION_H

#include <boost/asio/io_service.hpp>
#include "session.h"
#include "endpoint.h"
#include "tcp_channel.h"
#include "bitcoin_parser.h"
#include <boost/shared_ptr.hpp>
#include <deque>

namespace bcus {

class bitcoin_session : public bcus::session {

    typedef boost::function<void (  bcus::bitcoin_session *session,
                                    bitcoin_decoder *d,
                                    const bcus::endpoint &)> READ_CALLBACK;

    typedef boost::function<void (  bcus::bitcoin_session *session,
                                    const bcus::endpoint &ep) > CLOSE_CALLBACK;

    typedef bitcoin_session this_type;
    typedef bcus::tcp_channel<bcus::bitcoin_session, bcus::bitcoin_decoder> channel_type;
    typedef boost::shared_ptr<channel_type> channel_ptr;

public:
    bitcoin_session( boost::asio::io_service &io_service, tcp_socket_ptr &socket);

    virtual ~bitcoin_session();

    template<typename Function>
    void set_read_callback(Function cb) {
        read_callback_ = cb;
    }
    template<typename Function>
    void set_close_callback(Function cb) {
        close_callback_ = cb;
    }

    /** called by channel */
    void on_read(bcus::bitcoin_decoder *d, const bcus::endpoint &ep);
    void on_peer_close();

    void close();
    int  send(const void *buf, int len);

private:
    boost::asio::io_service  &io_service_;
    READ_CALLBACK  read_callback_;
    CLOSE_CALLBACK close_callback_;

    channel_ptr channel_;

    //bitcoin_requester_type  *bitcoin_requester_;
    //bitcoin_responser_type *bitcoin_responser_;
};


}

#endif
