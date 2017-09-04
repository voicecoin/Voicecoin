#ifndef BCUS_ASIO_RING_BUFFER_H
#define BCUS_ASIO_RING_BUFFER_H
#include <stdlib.h>
#include <cstring>

namespace bcus {

#define BUFFER_ALIGN(size)  (((size) + 2047) & ~2047) //SAP_ALIGN(size, 2048)
class ring_buffer {
public:
    const static unsigned int BUFFER_INIT_CAPACITY = 10240;
    ring_buffer(unsigned int capacity = BUFFER_INIT_CAPACITY) : begin_(0), end_(0), capacity_(capacity) {
        base_ = (char *)malloc(capacity);
    }
    void add_capacity() {
        add_capacity(capacity_);
    }
    void add_capacity(unsigned int nLeft) {
        capacity_ += nLeft;
        base_ = (char *)realloc(base_, capacity_);
    }
    void append(const char *p) {
        append(p, strlen(p));
    }
    void append(const char *p, unsigned int len) {
        if (len > capacity()) {
            add_capacity(len + capacity_);
        }
        if (len > right_capacity()) {
            memmove(base_, base_ + begin_, end_ - begin_);
            end_ = end_ - begin_;
            begin_ = 0;
        }
        memcpy(base_ + end_, p, len);
        end_ += len;
    }

    void   pop_front(unsigned int len)
    {
        begin_ += len;
        if (begin_ > end_) {
            begin_ = end_;
        }
    }
    bool            empty() { return size() == 0; }
    unsigned int    size()  { return end_ - begin_; }
    unsigned int    len()   { return end_ - begin_; }
    char *          base() const { return base_; }
    char *          head() const { return base_ + begin_; }
    char *          top()  const { return base_ + end_; }
    unsigned int    capacity() const { return capacity_ - end_ + begin_; }
    unsigned int    right_capacity() const { return capacity_ - end_; }
    ~ring_buffer() { free(base_); }

private:
    char            *base_;
    unsigned int    begin_;
    unsigned int    end_;
    unsigned int    capacity_;
};

}
#endif


