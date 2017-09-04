#ifndef UPNP_LOADER_H
#define UPNP_LOADER_H

#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/miniwget.h>
#include <miniupnpc/upnpcommands.h>
#include <miniupnpc/upnperrors.h>
#include <boost/thread.hpp>
#include "singleton.h"

namespace bcus {

class upnp_loader_thread : public singleton<upnp_loader_thread> {
    friend class singleton<upnp_loader_thread>;
public:
    void init(const char *name, int port = 8008);
    void start();
    void stop();
    void join();
private:
    void init_resource();
    void do_upnp_mapping();
private:
    std::string port_;
    std::string name_;
    boost::thread thread_;

    struct UPNPDev *dev_;
    struct UPNPUrls urls_;
    struct IGDdatas data_;
    char lanaddr_[64];
    char externip_[64];
};

}







#endif
