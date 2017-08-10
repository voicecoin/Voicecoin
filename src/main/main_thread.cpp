#include "main_thread.h"
#include <boost/bind.hpp>
#include "wallet.h"

main_thread &main_thread::instance()
{
    static main_thread t;
    return t;
}

void main_thread::start()
{
    work_ = new boost::asio::io_service::work(io_service_);
    thread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
}

void main_thread::stop()
{
    delete work_;
    thread_.join();
}

void main_thread::start_mining()
{
    io_service_.post(boost::bind(&main_thread::do_start_mining, this));
}

void main_thread::do_start_mining()
{
    block *blk = block_chain::instance().prepare_block();
    thread_mining_ = boost::thread(boost::bind(&main_thread::mining_thread, this, blk));
}

void main_thread::mining_thread(block *blk)
{
    block_chain::instance().generate_block(blk);
}

void main_thread::accept_new_block(block *blk)
{
    io_service_.post(boost::bind(&main_thread::do_accept_new_block, this, blk));
}

void main_thread::do_accept_new_block(block *blk)
{
    block_chain::instance().stop_generate_block();
    thread_mining_.join();
    block_chain::instance().accept_block(blk);
    do_start_mining();
}

void main_thread::get_balance()
{
    io_service_.post(boost::bind(&main_thread::do_get_balance, this));
}

void main_thread::do_get_balance()
{
    wallet::instance().get_balance();
}

#if 0
msg_queue::msg_queue(int size)
    : capacity_(size)
    , size_(0)
    , begin_(0)
    , end_(0)
{
    datas_ = new void*[size];
}

msg_queue::~msg_queue()
{
    delete[] datas_;
}

void msg_queue::push(void *data)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (size_ == capacity_)
        cond_push_.wait(lock);
    datas_[end_++] = data;
    if (end_ == capacity_)
        end_ = 0;
    if (size_++ == 0)
        cond_pop_.notify_one();
}

void *msg_queue::pop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (size_ == 0)
        cond_pop_.wait(lock);
    void *data = datas_[begin_++];
    if (begin_ == capacity_)
        begin_ = 0;
    if (size_-- == capacity_)
        cond_push_.notify_one();
    return data;
}

//////////////////////////////////////////////////////////////////////////

void msg_thread::start()
{
    thread_ = std::thread(std::bind(&msg_thread::run, this));
}

void msg_thread::stop()
{

}

void msg_thread::run()
{
    while (true)
    {
        void *data = pop();
    }
}
#endif
