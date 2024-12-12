#include "net_main.h"
#include "upnp_loader_thread.h"

namespace bcus {

void net_main::start() {
    bcus::upnp_loader_thread::get_instance()->init("chain domain", CC_PORT);
    bcus::upnp_loader_thread::get_instance()->start();
}

void net_main::start_server() {

}


}
