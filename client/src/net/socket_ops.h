#ifndef BCUS_SOCKET_OPS_H
#define BCUS_SOCKET_OPS_H

#include "platform.h"
#include "error_code.h"

namespace bcus {

class socket_ops {
public:
    static int gethostbyname(const char *host_name, char *outip, error_code& ec);
    static int get_interface_ip(const char *interface1, char *outip, error_code& ec);
    static int get_local_ip(const char *mask, char *outip, error_code& ec);
};

}

#endif
