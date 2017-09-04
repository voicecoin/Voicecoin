#ifndef BCUS_NET_CC_CLIENT_H
#define BCUS_NET_CC_CLIENT_H

#include <boost/unordered_map.hpp>
#include "stream_connector.h"
#include "cc_session.h"

namespace bcus {

class cc_client {
    typedef boost::function<void (  uint32_t session_id, const bcus::endpoint &ep )> CONNECTED_CALLBACK;
    typedef boost::function<void ( uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep )> READ_CALLBACK;
public:
    cc_client(boost::asio::io_service &io_service);

    template<typename Function>
    void set_connected_callback(Function cb) { connect_callback_ = cb; }

    template<typename Function>
    void set_read_callback(Function cb) { read_callback_ = cb; }

    void register_addr(const char *addr, int port);

    void send_to_all(const void *buf, int len);
    void send(uint32_t session_id, const void *buf, int len);

protected:
    void on_read(  bcus::cc_session *session,
                   cc_decoder *d,
                   const bcus::endpoint &);

    void on_peer_close( bcus::cc_session *session,
                        const bcus::endpoint &ep);
private:
    void do_connect(const std::string &addr, int port);
    tcp_socket_ptr create_socket() {
        return tcp_socket_ptr(new tcp_socket(io_service_));
    }
    void connect_result(tcp_socket_ptr &socket,
                        const bcus::endpoint &ep,
                        const bcus::error_code &err,
                        void *context);

private:
    boost::asio::io_service &io_service_;
    typedef std::map<uint32_t, cc_session *> SESSION_MAP;
    SESSION_MAP map_session_;

    bcus::stream_connector connector_;

    CONNECTED_CALLBACK connect_callback_;
    READ_CALLBACK read_callback_;
};

}

#endif
