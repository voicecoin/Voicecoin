#ifndef BCUS_HTTP_SERVER_H
#define BCUS_HTTP_SERVER_H

#include <boost/asio.hpp>
#include "stream_acceptor.h"
#include "http_session_container.h"

namespace bcus {

class http_server {
    typedef boost::function<void (  uint32_t containerid,
                                    uint32_t sessionid,
                                    http_decoder *d,
                                    const bcus::endpoint &ep)> READ_CALLBACK;
public:
    http_server(boost::asio::io_service &io_service);
    http_server(std::vector<boost::asio::io_service *> vec_io_service);
    ~http_server();

    /** set callback in all thread */
    template<typename Function>
    void set_read_callback(Function cb) {
        READ_CALLBACK callback = cb;
        do_set_read_callback(callback);
    }

    /** set callback in one thread
      * id is the index of std::vector<boost::asio::io_service *>
      * id is between 0 to vec_io_service.size()
    */
    template<typename Function>
    void set_read_callback(uint32_t id, Function cb) {
        READ_CALLBACK callback = cb;
        do_set_read_callback(id, callback);
    }

    int  register_addr(const bcus::endpoint &ep);

    int  send_response(uint32_t containerid, uint32_t sessionid, uint32_t seqno, const void *buf, int len);


private:
    void init_acceptor(boost::asio::io_service &io_service);
    void do_set_read_callback(READ_CALLBACK cb);
    void do_set_read_callback(uint32_t id, READ_CALLBACK cb);

    http_session_container *get_container();
    http_session_container *get_container(const boost::asio::io_service &io_service);

    tcp_socket_ptr create_socket();
    void on_accepted(tcp_socket_ptr &socket);

private:
    uint32_t current_conatiner_;
    std::vector<http_session_container *> vec_session_container_;

    stream_acceptor *acceptor_;
};

}

#endif
