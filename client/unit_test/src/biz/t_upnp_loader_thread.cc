#include <gtest/gtest.h>

#include "loghelper.h"
#include "upnp_loader_thread.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>


TEST(biz_upnp_loader_thread, start) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    bcus::upnp_loader_thread::get_instance()->init("test nginx", 8008);
    bcus::upnp_loader_thread::get_instance()->start();
    bcus::upnp_loader_thread::get_instance()->join();
}


TEST(biz_upnp_loader_thread, stop) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    bcus::upnp_loader_thread::get_instance()->init("test nginx", 8008);
    bcus::upnp_loader_thread::get_instance()->stop();
}
