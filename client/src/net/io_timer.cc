#include "io_timer.h"
#include "loghelper.h"
#include <boost/bind.hpp>

namespace bcus {

io_timer::~io_timer()
{
    if (timer_)
        delete timer_;
    //XLOG(XLOG_DEBUG, "io_timer::%s, interval[%u], entype_[%d], status[%u]\n", __FUNCTION__, dwinterval_, entype_, status_);
}
bool io_timer::start()
{
    stop();
    status_ = WAITING;
    timer_->expires_from_now(boost::posix_time::milliseconds(dwinterval_));
    timer_->async_wait(make_alloc_handler(alloc_,
                boost::bind(&io_timer::callback, this, boost::asio::placeholders::error)));
    return true;
}
void io_timer::stop()
{
    status_ = USER_CANCLE;
    if (timer_) {
        boost::system::error_code ignore_ec;
        timer_->cancel(ignore_ec);
    }
}
void io_timer::callback(const boost::system::error_code& err)
{
    if (status_ == USER_CANCLE) {
        return;
    }
    if (type_ == TIMER_CIRCLE) {
        start();
    }
    if (status_ == WAITING) {
        status_ = TIME_OUT;
    }
    callback_(); //must at the last line; timer may be deleted in callback_();
}

}
