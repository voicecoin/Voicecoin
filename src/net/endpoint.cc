#include "endpoint.h"
#include "error_code.h"
#include "socket_ops.h"

#define BCUS_OS_UNIX_PATH_MAX 108

namespace bcus {

endpoint::endpoint()
{
    memset(&data_, 0, sizeof(data_));
}
endpoint::endpoint(unsigned short port_num)
{
    memset(&data_, 0, sizeof(data_));
    data_.v4.sin_family = AF_INET;
    data_.v4.sin_port = htons(port_num);
    data_.v4.sin_addr.s_addr = INADDR_ANY;
}
endpoint::endpoint(const char *ip, unsigned short port_num)
{
    memset(&data_, 0, sizeof(data_));
    data_.v4.sin_family = AF_INET;
    data_.v4.sin_port = htons(port_num);
    data_.v4.sin_addr.s_addr = inet_addr(ip);
}
endpoint::endpoint(const char *path)
{
    memset(&data_, 0, sizeof(data_));
    //data_.un.sun_family = AF_UNIX;
    //int len = (strlen(path) > BCUS_OS_UNIX_PATH_MAX) ? BCUS_OS_UNIX_PATH_MAX : strlen(path);
    //memcpy(data_.un.sun_path, path, len);
}
endpoint::endpoint(const endpoint& other) : data_(other.data_)
{
}
endpoint& endpoint::operator=(const endpoint& other)
{
    data_ = other.data_;
    return *this;
}
void endpoint::ip(const char *ip)
{
    data_.v4.sin_family = AF_INET;
    data_.v4.sin_addr.s_addr = inet_addr(ip);
}
void endpoint::port(unsigned short port_num)
{
    data_.v4.sin_port = htons(port_num);
}
void endpoint::path(const char *path)
{
    //data_.un.sun_family = AF_UNIX;
    //int len = (strlen(path) > BCUS_OS_UNIX_PATH_MAX) ? BCUS_OS_UNIX_PATH_MAX : strlen(path);
    //memcpy(data_.un.sun_path, path, len);
}
std::string endpoint::to_string() const
{
    if (is_v4()) {
        char szaddr[64] = {0};
        snprintf(szaddr, sizeof(szaddr) -1, "%s:%u", inet_ntoa(data_.v4.sin_addr), ntohs(data_.v4.sin_port));
        return std::string(szaddr);
    } else if (is_un()) {
        //return std::string(data_.un.sun_path);
    } else {
        return "";
    }
}

static inline int check_ip(const char *ip) {
    int v1, v2, v3, v4;
    if (4 != sscanf(ip, "%d.%d.%d.%d", &v1, &v2, &v3, &v4)) {
        return -1;
    }
    if (v1 < 0 || v1 > 255 || v2 < 0 || v2 > 255 || v3 < 0 || v3 > 255 || v4 < 0 || v4 > 255) {
        return -1;
    }
    return 0;
}
static inline int check_port(unsigned int port) {
    return (port > 65535) ? -2 : 0;
}
static inline bool is_broad_cast(const char *ip) {
    int v1, v2, v3, v4;
    if (4 != sscanf(ip, "%d.%d.%d.%d", &v1, &v2, &v3, &v4)) {
        return false;
    }
    return v4 == 255;
}
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
int endpoint::init(const char *addr, endpoint *out_local_endpoint) {
    char sz_type[32] = {0};
    char multi_local[32] = {0};
    char host[256] = {0};
    unsigned int port = 0;

    /** addr is multi://hostmask@ip:port */
    if ( 3 == sscanf(addr, "multi://%31[^@]@%255[^:]:%u", multi_local, host, &port)) {
        if ((0 != check_ip(host)) || (0 != check_port(port))) {
            return -1;
        }

        this->ip(host);
        this->port(port);

        char sz_local_ip[32] = {0};
        error_code ec;
        if (0 != socket_ops::get_local_ip(multi_local, sz_local_ip, ec)) {
            return -1;
        }

        if (out_local_endpoint) {
            out_local_endpoint->ip(sz_local_ip);
            out_local_endpoint->port(port);
        }
        return MULTICAST;
    }

    /** addr is protocol://ip:port */
    int ret = sscanf(addr, "%31[^:]://%[^:]:%u", sz_type, host, &port);
    if (ret != 3 && ret != 2) {
        return -1;
    }

    int type = 0;
    if (0 == strcasecmp("tcp", sz_type)) {
        type = TCP;
    } else if (0 == strcasecmp("unix", sz_type)) {
        type = UNIX;
    } else if (0 == strcasecmp("udp", sz_type)) {
        type = is_broad_cast(host) ? BROADCAST : UDP;
    } else if (0 == strcasecmp("multi", sz_type)) {
        type = MULTICAST;
    } else {
        return -1;
    }

    if (UNIX == type) {
        this->path(host);
        return UNIX;
    }

    if ((0 != check_ip(host)) || (0 != check_port(port))) {
        return -1;
    }

    this->ip(host);
    this->port(port);

    if (out_local_endpoint) {
        out_local_endpoint->ip("0.0.0.0");
        out_local_endpoint->port(port);
    }


    return type;
}

}
