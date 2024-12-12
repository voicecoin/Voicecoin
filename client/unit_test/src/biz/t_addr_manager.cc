#include <gtest/gtest.h>

#include "loghelper.h"
#include "addr_manager.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

static void extern_ip_callback(const std::string &ip) {
    XLOG(XLOG_DEBUG, "%s, ip[%s]\n", __FUNCTION__, ip.c_str());
}

TEST(biz_addr_manager, extern_ip) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    boost::asio::io_service io_service;
    bcus::addr_manager m(io_service);
    m.get_extern_ip(extern_ip_callback);

    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    thread.join();
}

