#ifndef BCUS_FIB_REQ_SESSION_H
#define BCUS_FIB_REQ_SESSION_H

#include <boost/asio/io_service.hpp>
#include "session.h"
#include "endpoint.h"
#include "tcp_channel.h"
#include "http_parser.h"
#include <boost/shared_ptr.hpp>
#include <deque>

namespace bcus {

class http_session : public bcus::session {

    typedef boost::function<void (  bcus::http_session *session,
                                    http_decoder *d,
                                    const bcus::endpoint &,
                                    void *context)> READ_CALLBACK;

    typedef boost::function<void (  bcus::http_session *session,
                                    const bcus::endpoint &ep) > CLOSE_CALLBACK;

    typedef http_session this_type;
    typedef bcus::tcp_channel<bcus::http_session, bcus::http_decoder> channel_type;
    typedef boost::shared_ptr<channel_type> channel_ptr;
    class http_requester_type;
    class http_responser_type;

public:
    http_session( boost::asio::io_service &io_service, tcp_socket_ptr &socket);

    virtual ~http_session();

    template<typename Function>
    void set_read_callback(Function cb) {
        read_callback_ = cb;
    }
    template<typename Function>
    void set_close_callback(Function cb) {
        close_callback_ = cb;
    }

    void set_domain(const char *domain) {
        domain_ = domain;
    }
    const std::string &get_domain() {
        return domain_;
    }

    /** called by channel */
    void on_read(bcus::http_decoder *d, const bcus::endpoint &ep);
    void on_peer_close();

    void close();
    int  do_request(uint32_t seqno, const void *buf, int len, void *context = NULL);
    int  do_response(uint32_t seqno, const void *buf, int len);

private:
    virtual int send(const void *buf, int len) { return -1; }

private:
    boost::asio::io_service  &io_service_;
    READ_CALLBACK  read_callback_;
    CLOSE_CALLBACK close_callback_;

    channel_ptr channel_;

    http_requester_type  *http_requester_;
    http_responser_type *http_responser_;

    std::string domain_;
};


}

#endif
