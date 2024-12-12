#ifndef CC_SERVER_THREAD_H
#define CC_SERVER_THREAD_H

#include "loghelper.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "cc_server.h"

namespace bcus {
class cc_server_thread : public bcus::singleton<cc_server_thread> {
    typedef boost::function<void ( uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep )> READ_CALLBACK;

public:
    cc_server_thread();
    void start();
    void join();

    void start_server(const char *ip, int port);

    template<typename Function>
    void set_read_callback(Function cb) { callback_ = cb; }

    void send_to_all(const void *buf, int len);
    void send(uint32_t session_id, const void *buf, int len);

private:
    void on_read(  uint32_t containerid, uint32_t sessionid, cc_decoder *d, const bcus::endpoint &ep);

    void do_send_version(uint32_t session_id, const bcus::endpoint &ep);
    void do_send_verback(uint32_t session_id, const bcus::endpoint &ep);

    void do_recv_version(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep);
    void do_recv_verback(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep);
private:
    boost::asio::io_service io_service_;
    boost::thread thread_;
    cc_server server_;

    READ_CALLBACK callback_;

    cc_encoder en_;
};

}
#endif
