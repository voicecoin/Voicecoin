#include "msg_thread.h"

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