#ifndef BCUS_MSG_THREAD_H
#define BCUS_MSG_THREAD_H

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

#endif // BCUS_MSG_THREAD_H
