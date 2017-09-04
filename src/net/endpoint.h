#ifndef BCUS_ENDPOINT_H
#define BCUS_ENDPOINT_H

#include <boost/asio/detail/socket_types.hpp>
#include "platform.h"
#include <string>

namespace bcus {

typedef boost::asio::detail::socket_type socket_handle;

class endpoint
{
public:
    enum { TCP = 0X0100, UDP = 0X0200, UNIX = 0X0300, MULTICAST = 0X0201, BROADCAST = 0X0202 };
    endpoint();
    endpoint(unsigned short port_num);
    endpoint(const char *ip, unsigned short port_num);
    endpoint(const char *path);
    endpoint(const endpoint& other);
    endpoint& operator=(const endpoint& other);
    void ip(const char *ip);
    void port(unsigned short port_num);
    void path(const char *path);

    inline boost::asio::detail::socket_addr_type* data();
    inline const boost::asio::detail::socket_addr_type* data() const;
    inline std::size_t size() const;
    inline void family(int sa_family);
    inline bool is_v4() const;
    inline bool is_un() const;
    inline bool empty() const;
    inline const char *ip() const;
    inline uint32_t ip_data() const;
    inline uint16_t port() const;

    /**
    *  addr : tcp://ip:port;            eg:  tcp://127.0.0.1:10010
    *         udp://ip:port;            eg:  udp://127.0.0.255:10010
    *         unix://port;              eg:  unix://tmp/test.socket
    *         multi://hostmask@ip:port  eg:  multi://197.168.1@224.0.0.2:10010, need to set out_local_endpoint
    *         multi://ip:port           eg:  multi://224.0.0.2:10010
    *  out_local_endpoint : if addr is multi://hostmask@ip:port, set local ip mask to out_local_endpoint
    *  return : success, return protocol type
    *           fail,    return -1
    */
    int init(const char *addr, endpoint *out_local_endpoint = NULL);
    std::string to_string() const;


private:
    union data_union
    {
        boost::asio::detail::socket_addr_type  base;
        boost::asio::detail::sockaddr_in4_type v4;
        //boost::asio::detail::sockaddr_in6_type v6;
        //boost::asio::detail::sockaddr_un_type un;
    } data_;
};

inline boost::asio::detail::socket_addr_type* endpoint::data() {
    return &data_.base;
}
inline const boost::asio::detail::socket_addr_type* endpoint::data() const {
    return &data_.base;
}
inline std::size_t endpoint::size() const {
    if (is_v4()) {
        return sizeof(boost::asio::detail::sockaddr_in4_type);
    } else if (is_un()) {
        //return sizeof(boost::asio::detail::sockaddr_un_type);
    } else {
        return sizeof(boost::asio::detail::sockaddr_in6_type);
    }
}
inline void endpoint::family(int sa_family) {
    data_.base.sa_family = sa_family;
}
inline bool endpoint::is_v4() const {
    return data_.base.sa_family == AF_INET;
}
inline bool endpoint::is_un() const {
    return data_.base.sa_family == AF_UNIX;
}
inline bool endpoint::empty() const {
    if (is_v4()) {
        return ntohs(data_.v4.sin_port) == 0;
    } else if (is_un()) {
        //return *(data_.un.sun_path) == 0;
    } else {
        return true;
    }
}
inline const char *endpoint::ip() const {
    if (is_v4()) {
        return inet_ntoa(data_.v4.sin_addr);
    } else if (is_un()) {
        //return data_.un.sun_path;
    } else {
        return "";
    }
}
inline uint32_t endpoint::ip_data() const {
    uint32_t v;
    memcpy(&v, &(data_.v4.sin_addr), sizeof(v));
    return v;
}
inline uint16_t endpoint::port() const {
    return (is_v4()) ? ntohs(data_.v4.sin_port) : 0;
}

}
#endif
