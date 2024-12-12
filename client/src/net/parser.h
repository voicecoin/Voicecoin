#ifndef BCUS_NET_PARSER_H
#define BCUS_NET_PARSER_H

#include <vector>
#include "buffer.h"
#include "platform.h"

namespace bcus {

class decoder {
public:
    decoder() : buf_(NULL), len_(0) { }
    virtual ~decoder() {}
    /** return value p
        ret = -1 //error
        ret = 0  incomplete packet, continue to read
        ret = len //complete
        ret < buf + len  //more than one packet; len is the first packet's size
    */
    virtual int decode (const char *buf, int len) { return len; }
    virtual const char *get_buf() const { return buf_; }
    virtual int get_len() const { return len_; }
protected:
    const char *buf_;
    int len_;
};

class encoder {
public:
    encoder() {}
    encoder(int buf_len) : buffer_(buf_len) {}
    virtual ~encoder() {}
    bcus::buffer *get_buffer() { return &buffer_; }

    virtual void        encode() = 0;
    virtual const char *get_buf() const { return buffer_.base(); }
    virtual int         get_len() const { return buffer_.len(); }
protected:
    bcus::buffer buffer_;
};

}
#endif

