#ifndef BCUS_NET_CHANNEL_FACTORY_H
#define BCUS_NET_CHANNEL_FACTORY_H

#include "tcp_channel.h"
//#include <cffex/net/unix_channel.h>
//#include <cffex/net/udp_channel.h>

namespace bcus {

class channel_factory {
public:
    template<typename owner_type, typename decoder>
    static channel *create( int type,
                            boost::asio::io_service &io_service,
                            owner_type *owner,
                            const bcus::socket_handle &fd,
                            uint32_t status_check_sec = 0) {
        switch (type) {
          case bcus::endpoint::TCP :
            return new tcp_channel<owner_type, decoder>(io_service, owner, fd, status_check_sec);
          /*
          case cbcus::endpoint::UNIX :
            return new unix_channel<owner_type, decoder>(io_service, owner, fd, status_check_sec);
          case bcus::endpoint::BROADCAST :
            return new udp_channel<owner_type, decoder>(io_service, owner, fd, status_check_sec);
          case bcus::endpoint::MULTICAST :
            return new udp_channel<owner_type, decoder>(io_service, owner, fd, status_check_sec);
          case bcus::endpoint::UDP :
            return new udp_channel<owner_type, decoder>(io_service, owner, fd, status_check_sec);
          */
          default:
            break;
        }
        return NULL;
    }
};

}

#endif
