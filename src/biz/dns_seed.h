#ifndef CBUS_DNS_SEED_H
#define CBUS_DNS_SEED_H

#include <boost/asio.hpp>
#include "endpoint.h"

namespace bcus {

class dns_seed {
public:
    dns_seed(boost::asio::io_service &io_service) : resolver_(io_service) {
    }

    std::vector<bcus::endpoint> resolve(const char *addr, const char *port);

private:
    boost::asio::ip::tcp::resolver resolver_;
};

}
#endif
