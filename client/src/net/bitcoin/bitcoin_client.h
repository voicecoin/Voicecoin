#ifndef BCUS_NET_BITCOIN_CLIENT_H
#define BCUS_NET_BITCOIN_CLIENT_H

#include <boost/unordered_map.hpp>
#include "net/stream_connector.h"
#include "bitcoin_session.h"

namespace bcus {

class bitcoin_client {
    typedef boost::function<void (  bcus::bitcoin_decoder *d )> READ_CALLBACK;
public:
    bitcoin_client(boost::asio::io_service &io_service);

    template<typename Function>
    void set_read_callback(Function cb) {
        callback_ = cb;
    }

    void register_addr(const char *addr, int port);

protected:
    void on_read(  bcus::bitcoin_session *session,
                   bitcoin_decoder *d,
                   const bcus::endpoint &);

    void on_peer_close( bcus::bitcoin_session *session,
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


    void do_send_version(bitcoin_session *session, const bcus::endpoint &ep);
    void do_send_verback(bitcoin_session *session, const bcus::endpoint &ep);

    void do_recv_version(bitcoin_session *session, bitcoin_decoder *d, const bcus::endpoint &ep);
    void do_recv_verback(bitcoin_session *session, bitcoin_decoder *d, const bcus::endpoint &ep);

private:
    boost::asio::io_service &io_service_;
    typedef std::map<uint32_t, bitcoin_session *> SESSION_MAP;
    SESSION_MAP map_session_;

    bcus::stream_connector connector_;
    READ_CALLBACK callback_;

    bitcoin_encoder en_;
};

}

#endif
