#include "dns_seed.h"
#include "loghelper.h"

namespace bcus {

std::vector<bcus::endpoint> dns_seed::resolve(const char *addr, const char *port)
{
    XLOG(XLOG_DEBUG, "dns_seed::%s, addr[%s]\n", __FUNCTION__, addr);
    std::vector<bcus::endpoint> vec;

    boost::asio::ip::tcp::resolver::query query(addr, port);
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver::iterator end;
    boost::asio::ip::tcp::resolver::iterator itr = resolver_.resolve(query, ec);
    if (ec) {
        XLOG(XLOG_WARNING, "dns_seed::%s, addr[%s], resolve failed[%d-%s]\n", __FUNCTION__, addr, ec, ec.message().c_str());
        return vec;
    }

    while (itr != end) {
        boost::asio::ip::tcp::endpoint ep = *itr++;
        vec.push_back(bcus::endpoint(ep.address().to_string().c_str(), ep.port()));
        XLOG(XLOG_DEBUG, "dns_seed::%s,     ip[%s], port[%d]\n", __FUNCTION__, ep.address().to_string().c_str(), ep.port());
    }
    return vec;
}

}