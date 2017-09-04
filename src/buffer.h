#ifndef _BCUS_COMMON_BUFFER_H_
#define _BCUS_COMMON_BUFFER_H_
#include <stdlib.h>
#include <cstring>
#include <string>

namespace bcus {

#define BUFFER_ALIGN(size)  (((size) + 2047) & ~2047) //SAP_ALIGN(size, 2048)
class buffer {
public:
    const static unsigned int BUFFER_INIT_CAPACITY = 10240;
    buffer(unsigned int capacity = BUFFER_INIT_CAPACITY) : capacity_(capacity), loc_(0) {
        base_ = (char *)malloc(capacity);
    }
    void add_capacity() {
        capacity_ += capacity_;
        base_ = (char *)realloc(base_, capacity_);
    }
    void add_capacity(unsigned int nLeft) {
        capacity_ += nLeft + nLeft;
        base_ = (char *)realloc(base_, capacity_);
    }
    buffer &append(const char *p) {
        append(p, strlen(p));
        return *this;
    }
    buffer &append(const std::string &str) {
        append(str.c_str(), str.length());
        return *this;
    }
    buffer &append(const char *p, unsigned int len) {
        if (loc_ + len > capacity_) {
            add_capacity(len + capacity_);
        }
        memcpy(base_ + loc_, p, len);
        loc_ += len;
        return *this;
    }
    char * base() const {return base_;}
    char * top() const {return base_ + loc_;}
    void inc_loc(unsigned int loc){loc_ += loc;}
    void reset_loc(unsigned int loc){loc_ = loc;}
    unsigned int capacity() const {return capacity_-loc_;}
    unsigned int len() const{return loc_;}
    ~buffer(){free(base_);}
private:
    buffer(const buffer &);  //noncopyable
    char *base_;
    unsigned int capacity_;
    unsigned int loc_;
};

}
#endif


