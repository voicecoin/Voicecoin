#ifndef BCUS_MAIN_THREAD_H
#define BCUS_MAIN_THREAD_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "blockchain.h"

class main_thread
{
public:
    static main_thread &instance();

    void start();
    void stop();

    void start_mining();
    void accept_new_block(block *blk);
    void get_balance();
    void show_wallet();

private:
    main_thread() {}

    void do_start_mining();
    void mining_thread(block *blk);
    void do_accept_new_block(block *blk);
    void do_get_balance();
    void do_show_wallet();

private:
    boost::asio::io_service io_service_;
    boost::asio::io_service::work *work_;
    boost::thread thread_;
    boost::thread thread_mining_;
};

#if 0
#include <mutex>
#include <condition_variable>

class msg_queue
{
public:
    msg_queue(int size = 102400);
    ~msg_queue();

    void push(void *data);
    void *pop();

private:
    std::mutex mutex_;
    std::condition_variable cond_push_;
    std::condition_variable cond_pop_;
    void **datas_;
    int capacity_;
    int size_;
    int begin_;
    int end_;
};

class msg_thread : public msg_queue
{
public:
    void start();
    void stop();

private:
    void run();

private:
    std::thread thread_;
};

#endif

#endif // BCUS_MAIN_THREAD_H
