#include <gtest/gtest.h>

#include "loghelper.h"
#include "io_timer.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

static void timeout_callback(int i, bcus::io_timer *t) {
    XLOG(XLOG_DEBUG, "%s, i[%d]\n", __FUNCTION__, i);
    EXPECT_EQ(t->status(), bcus::io_timer::TIME_OUT);
    static int count = 0;
    if (count++ > 5) {
        t->stop();
    }
}

TEST(net_io_timer, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    boost::asio::io_service io_service;
    bcus::io_timer timer1;
    timer1.init(&io_service, 1000,
        boost::bind(timeout_callback, 1, &timer1), bcus::io_timer::TIMER_ONCE);
    timer1.start();

    bcus::io_timer timer2;
    timer2.init(&io_service, 1000,
        boost::bind(timeout_callback, 2, &timer2), bcus::io_timer::TIMER_CIRCLE);
    timer2.start();

    boost::thread thread(boost::bind(&boost::asio::io_service::run, &io_service));
    thread.join();
}

