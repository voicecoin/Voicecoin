#include <gtest/gtest.h>

#include "loghelper.h"
#include "cc_client_thread.h"
#include <boost/bind.hpp>
#include <boost/thread.hpp>

static void cc_client_read(uint32_t session_id, bcus::cc_decoder *d, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "%s, session_id[%d], ep[%s]%s\n", __FUNCTION__,
        session_id, ep.to_string().c_str(), d->to_string().c_str());
}

TEST(cc_client_thread, t1) {
    XLOG(XLOG_DEBUG, "%s\n", typeid(this).name());

    bcus::cc_client_thread::get_instance()->set_read_callback(cc_client_read);
    bcus::cc_client_thread::get_instance()->register_addr("127.0.0.1", 47333);
    bcus::cc_client_thread::get_instance()->start();
    bcus::cc_client_thread::get_instance()->join();
}
