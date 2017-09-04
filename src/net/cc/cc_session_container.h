#ifndef BCUS_NET_CC_SESSION_CONTAINER_H
#define BCUS_NET_CC_SESSION_CONTAINER_H

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include "cc_session.h"

namespace bcus {

class cc_session_container
{
    typedef boost::function<void (  uint32_t containerid,
                                    uint32_t sessionid,
                                    cc_decoder *d,
                                    const bcus::endpoint &ep)> READ_CALLBACK;
public:
    cc_session_container(uint32_t id, boost::asio::io_service &io_service);
    ~cc_session_container();

    boost::asio::io_service &get_io_service() { return io_service_; }
    void do_accepted(tcp_socket_ptr &socket);

    /** session_callback */
    void on_read(cc_session *session, cc_decoder *d, const bcus::endpoint &ep);
    void on_peer_close(cc_session *session, const bcus::endpoint &ep);

    int send_to_all(const void *buf, int len);
    int send(uint32_t sessionid, const void *buf, int len);

    void set_callback(READ_CALLBACK cb) {
        callback_ = cb;
    }

private:
    uint32_t id_;
    boost::asio::io_service &io_service_;

    typedef std::map<uint64_t, cc_session *> SesstionMap;
    SesstionMap map_session_;

    READ_CALLBACK callback_;
};

}

#endif

