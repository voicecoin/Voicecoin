#ifndef CC_CLIENT_THREAD_H
#define CC_CLIENT_THREAD_H

#include "loghelper.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "net/cc/cc_client.h"
#include "net/cc/cc_parser.h"

namespace bcus {
class cc_client_thread : public bcus::singleton<cc_client_thread> {
    typedef boost::function<void ( uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep )> READ_CALLBACK;
public:
    cc_client_thread();
    void start();
    void join();
    void register_addr(const char *addr, int port);

    template<typename Function>
    void set_read_callback(Function cb) { callback_ = cb; }

    void send_to_all(const void *buf, int len);
    void send(uint32_t session_id, const void *buf, int len);

private:
    void on_read(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep);
    void on_connected(uint32_t session_id, const bcus::endpoint &ep);


    void do_send_version(uint32_t session_id, const bcus::endpoint &ep);
    void do_send_verback(uint32_t session_id, const bcus::endpoint &ep);

    void do_recv_version(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep);
    void do_recv_verback(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep);

private:
    boost::asio::io_service io_service_;
    boost::thread thread_;
    cc_client client_;

    READ_CALLBACK callback_;

    cc_encoder en_;
};

}
#endif
