#ifndef BCUS_NET_HTTP_CLIENT_H
#define BCUS_NET_HTTP_CLIENT_H

#include <boost/unordered_map.hpp>
#include "stream_connector.h"
#include "http_session.h"

namespace bcus {

class http_client {
    typedef boost::function<void (  bcus::http_decoder *d,
                                    const bcus::endpoint &ep,
                                    void *context)> READ_CALLBACK;

    struct session_info;
public:
    http_client(boost::asio::io_service &io_service);

    template<typename Function>
    void set_read_callback(Function cb) {
        callback_ = cb;
    }

    int request(const char *url, void *context = NULL);

protected:
    void on_read(  bcus::http_session *session,
                   http_decoder *d,
                   const bcus::endpoint &,
                   void *context);

    void on_peer_close( bcus::http_session *session,
                        const bcus::endpoint &ep);
private:
    void do_connect(const char *domain, session_info *info);
    tcp_socket_ptr create_socket() {
        return tcp_socket_ptr(new tcp_socket(io_service_));
    }
    void connect_result(tcp_socket_ptr &socket,
                        const bcus::endpoint &ep,
                        const bcus::error_code &err,
                        void *context);
    void do_request(session_info *info,
        const char *domain, const char *path, void *context);
    void reset(session_info *info);

private:
    boost::asio::io_service &io_service_;
    typedef boost::unordered_map<std::string, session_info *> SESSION_MAP;
    SESSION_MAP map_session_;

    bcus::stream_connector connector_;
    READ_CALLBACK callback_;

    uint32_t req_seqno_;
    bcus::http_encoder en_;
};

}

#endif
