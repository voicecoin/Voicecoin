#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include "parser.h"
#include "slice.h"
#include <map>

namespace bcus {

class http_decoder : public bcus::decoder {
public:
    http_decoder();
    virtual ~http_decoder() { }
    virtual int decode(const char *buf, int len);

    const bcus::slice body() const { return bcus::slice(body_, body_len_); }

    bool is_keep_alive() const { return is_keep_alive_; }
    int  http_version()  const { return http_version_; }
    int  http_code()     const { return http_code_; }
    const char *http_method() const { return method_; }
    const char *url_path() const { return url_path_; }

    const bcus::slice head(const char *k) const;

    bool  is_request() const   { return is_request_; }
    bool  is_response() const  { return !is_request_; }
    bool  is_heartbeat() const { return false; }

    void      seqno(uint64_t seqno) { seqno_ = seqno; }
    uint64_t  seqno() const { return seqno_; }
    std::string to_string(int indent = 0) const;

private:
    int  decode_request(const char *buf, int len);
    int  decode_response(const char *buf, int len);
    int  parse_head(const char *buf, int len);
    void check_head(const bcus::slice &key, const bcus::slice &value);
    void reset();

private:
    uint64_t seqno_;

    bool is_request_;

    char method_[32];
    char url_path_[1024];

    int http_version_;
    int http_code_;
    char code_des_[64];

    bool is_keep_alive_;
    bool is_chunked_;

    const char *body_;
    int body_len_;

    typedef std::map<bcus::slice, bcus::slice> HEADER;
    HEADER header_;
    bcus::buffer buf_body_;
};

class http_encoder : public bcus::encoder {
public:
    virtual ~http_encoder() {}

    virtual void        encode();
    virtual const char *get_buf() const { return buffer_.base(); }
    virtual int         get_len() const { return buffer_.len(); }

    void init(const char *url, const char *method);
    void init(const char *domain, const char *path, const char *method);
    void init(int http_code);
    void head(const char *k, const char *v);
    void body(const char *body);
    void body(const char *body, int len);
private:
    bool has_body_;
};

}

#endif
