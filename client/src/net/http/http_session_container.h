#ifndef BCUS_NET_HTTP_SESSION_CONTAINER_H
#define BCUS_NET_HTTP_SESSION_CONTAINER_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include "http_session.h"

namespace bcus {

class http_session_container
{
    typedef boost::function<void (  uint32_t containerid,
                                    uint32_t sessionid,
                                    http_decoder *d,
                                    const bcus::endpoint &ep)> READ_CALLBACK;
public:
    http_session_container(uint32_t id, boost::asio::io_service &io_service);
    ~http_session_container();

    boost::asio::io_service &get_io_service() { return io_service_; }
    void do_accepted(tcp_socket_ptr &socket);

    /** session_callback */
    void on_read(http_session *session, http_decoder *d, const bcus::endpoint &ep, void *);
    void on_peer_close(http_session *session, const bcus::endpoint &ep);

    int send_response(uint32_t sessionid, uint32_t seqno, const void *buf, int len);

    void set_callback(READ_CALLBACK cb) {
        callback_ = cb;
    }

private:
    uint32_t id_;
    boost::asio::io_service &io_service_;

    typedef std::map<uint64_t, http_session *> SesstionMap;
    SesstionMap map_session_;

    READ_CALLBACK callback_;
};

}

#endif

