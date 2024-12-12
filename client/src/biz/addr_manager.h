#ifndef CBUS_ADDR_MANAGER_H
#define CBUS_ADDR_MANAGER_H

#include "net/http/http_client.h"

namespace bcus {

class addr_manager {
    typedef boost::function<void (std::string)> addr_callback_function;
public:
    addr_manager(boost::asio::io_service &io_service) : cc_(io_service) {
    }
    template<typename FUNC>
    void get_extern_ip(FUNC callback);

private:
    void http_read(bcus::http_decoder *d, const bcus::endpoint &ep, void *context);

private:
    http_client cc_;
    addr_callback_function extern_ip_callback_;
};

template<typename FUNC>
void addr_manager::get_extern_ip(FUNC callback) {
    extern_ip_callback_ = callback;
    cc_.set_read_callback(boost::bind(&addr_manager::http_read, this, _1, _2, _3));
    cc_.request("http://checkip.dyndns.org/", &extern_ip_callback_);
}

}
#endif
