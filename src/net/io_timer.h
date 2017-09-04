#ifndef BCUS_IO_TIMER_H
#define BCUS_IO_TIMER_H

#include <stdint.h>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include "handle_alloc.h"

namespace bcus {

class io_timer {
 public:
    enum timer_type { TIMER_ONCE = 0, TIMER_CIRCLE };
    enum timer_status { WAITING = 0, TIME_OUT, USER_CANCLE };
    io_timer() : status_(WAITING), timer_(NULL), data_(NULL)
    {
    }

    template <typename Func>
    io_timer(boost::asio::io_service *o, uint32_t milsec=0, Func f=0, timer_type t=TIMER_ONCE, void *data=NULL) :
            dwinterval_(milsec), type_(t), status_(WAITING), callback_(f), timer_(NULL), data_(data)
    {
        timer_ = new boost::asio::deadline_timer(*o, boost::posix_time::milliseconds(milsec));
    }

    template <typename Func>
    void init(boost::asio::io_service *o, uint32_t milsec=0, Func f=0, timer_type t=TIMER_ONCE, void *data=NULL)
    {
        timer_ = new boost::asio::deadline_timer(*o, boost::posix_time::milliseconds(milsec));
        dwinterval_     =   milsec;
        callback_       =   f;
        type_           =   t;
        data_           =   data;
    }
    bool start();
    void stop();
    int  status(){ return status_; }
    void data(void *data) { data_ = data; }
    void *data() const { return data_; }
    ~io_timer();

 public:
    void callback(const boost::system::error_code& err);
    void interval(uint32_t milsec) { dwinterval_ = milsec;}
    uint32_t interval() const { return dwinterval_;}

 private:
    uint32_t dwinterval_; //unit : ms
    timer_type type_;
    timer_status status_;

    boost::function<void ( )> callback_;
    boost::asio::deadline_timer *timer_;
    void *data_;
    handle_alloc alloc_;
};

}
#endif
