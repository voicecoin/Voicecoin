#ifndef NET_MAIN_H
#define NET_MAIN_H


#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include "loghelper.h"

#define CC_PORT 9333

namespace bcus {
class net_main : public bcus::singleton<net_main> {
public:
    void start();

private:
    void start_server();
};

}
#endif
