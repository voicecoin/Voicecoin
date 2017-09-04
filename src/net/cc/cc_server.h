#ifndef BCUS_CC_SERVER_H
#define BCUS_CC_SERVER_H

#include <boost/asio.hpp>
#include "stream_acceptor.h"
#include "cc_session_container.h"

namespace bcus {

class cc_server {
public:
    typedef boost::function<void (  uint32_t containerid,
                                    uint32_t sessionid,
                                    cc_decoder *d,
                                    const bcus::endpoint &ep)> READ_CALLBACK;

    cc_server(boost::asio::io_service &io_service);
    cc_server(std::vector<boost::asio::io_service *> vec_io_service);
    ~cc_server();

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

    int  send_to_all(const void *buf, int len);
    int  send(uint32_t containerid, uint32_t sessionid, const void *buf, int len);

private:
    void init_acceptor(boost::asio::io_service &io_service);
    void do_set_read_callback(READ_CALLBACK cb);
    void do_set_read_callback(uint32_t id, READ_CALLBACK cb);

    cc_session_container *get_container();
    cc_session_container *get_container(const boost::asio::io_service &io_service);

    tcp_socket_ptr create_socket();
    void on_accepted(tcp_socket_ptr &socket);

private:
    uint32_t current_conatiner_;
    std::vector<cc_session_container *> vec_session_container_;

    stream_acceptor *acceptor_;

    READ_CALLBACK callback_;
};

}

#endif
