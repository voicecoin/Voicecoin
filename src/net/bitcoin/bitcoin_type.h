#ifndef BITCOIN_TYPE_H
#define BITCOIN_TYPE_H

#include <stdint.h>
#include <string>
#include "platform.h"
#include "string_helper.h"
#include "endpoint.h"
#include "cipher.h"

namespace bcus {

template<typename VALUE_TYPE>
struct number_type {
    typedef number_type<VALUE_TYPE> this_type;

    VALUE_TYPE value;

    int size() { return sizeof(VALUE_TYPE); }

    number_type() { value = 0; }
    number_type(const VALUE_TYPE &v) : value(v) { }
    void set_value(VALUE_TYPE v) { value = v; }
    VALUE_TYPE  get_value() { return value; }
    VALUE_TYPE  get_value() const { return value; }

    this_type &operator = (const VALUE_TYPE &v) { value = v; return *this; }
};

struct int8_type : public number_type<int8_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = *((int8_t *)p); return sizeof(int8_t); }
    int output(char *p) { *(int8_t *)p = value; return sizeof(int8_t); }
    int8_type &operator = (const int8_t &v) { value = v; return *this; }
};
struct uint8_type : public number_type<uint8_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = *((uint8_t *)p); return sizeof(uint8_t); }
    int output(char *p) { *(uint8_t *)p = value; return sizeof(uint8_t); }
    uint8_type &operator = (const uint8_t &v) { value = v; return *this; }
};
struct int16_type : public number_type<int16_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = g_ntohs(*((int16_t *)p)); return sizeof(int16_t); }
    int output(char *p) { *(int16_t *)p = g_htons(value); return sizeof(int16_t); }
    int16_type &operator = (const int16_t &v) { value = v; return *this; }
};
struct uint16_type : public number_type<uint16_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = g_ntohs(*((uint16_t *)p)); return sizeof(uint16_t); }
    int output(char *p) { *(uint16_t *)p = g_htons(value); return sizeof(uint16_t); }
    uint16_type &operator = (const uint16_t &v) { value = v; return *this; }
};
struct int32_type : public number_type<int32_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = g_ntohl(*((int32_t *)p)); return sizeof(int32_t); }
    int output(char *p) { *(int32_t *)p = g_htonl(value); return sizeof(int32_t); }
    int32_type &operator = (const int32_t &v) { value = v; return *this; }
};
struct uint32_type : public number_type<uint32_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = g_ntohl(*((uint32_t *)p)); return sizeof(uint32_t); }
    int output(char *p) { *(uint32_t *)p = g_htonl(value); return sizeof(uint32_t); }
    uint32_type &operator = (const uint32_t &v) { value = v; return *this; }
};
struct int64_type : public number_type<int64_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = g_ntohll(*((int64_t *)p)); return sizeof(int64_t); }
    int output(char *p) { *(int64_t *)p = g_htonll(value); return sizeof(int64_t); }
    int64_type &operator = (const int64_t &v) { value = v; return *this; }
};
struct uint64_type : public number_type<uint64_t> {
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { value = g_ntohll(*((uint64_t *)p)); return sizeof(uint64_t); }
    int output(char *p) { *(uint64_t *)p = g_htonll(value); return sizeof(uint64_t); }
    uint64_type &operator = (const uint64_t &v) { value = v; return *this; }
};

struct time_type : uint64_type {
    uint64_type &operator = (const time_t &v) { value = (uint64_t)v; return *this; }
    std::string to_string() const {
        time_t t = value;
        struct tm local_tm;
        localtime_r(&t, &local_tm);

        char sz[64] = {0};
        snprintf(sz, sizeof(sz) - 1, "%04d-%02d-%02d %02d:%02d:%02d",
            local_tm.tm_year + 1900, local_tm.tm_mon + 1, local_tm.tm_mday,
            local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
        return sz;
    }
};

template <int LEN>
struct fix_char_type {
    typedef fix_char_type<LEN> this_type;

    uint8_t value[LEN];

    fix_char_type() {
        memset(value, 0, LEN);
    }
    int size() { return LEN; }
    uint64_t  get_value() { return value; }
    uint64_t  get_value() const { return value; }
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) { memcpy(value, p, LEN); return LEN; }
    int output(char *p) { memcpy(p, value, LEN); return LEN;}

    uint8_t &operator [](int index) {
        if (index >= LEN) {
            throw std::runtime_error("fix_char_type, outof range");
        }
        return value[index];
    }
    const uint8_t &operator [](int index) const {
        if (index >= LEN) {
            throw std::runtime_error("fix_char_type, outof range");
        }
        return value[index];
    }

    this_type &operator = (const char *p) {
        int len = strlen(p);
        if (len > LEN) { len = LEN; }
        memcpy(value, p, len);
        return *this;
    }
};
/*
< 0xfd          1   uint8_t
<= 0xffff       3   0xfd + uint16_t
<= 0xffffffff   5   0xfe + uint32_t
-               9   0xff + uint64_t
*/
struct var_int_type {
    uint64_t value;

    int  size() {
        if (value < 0xfd) {
            return 1;
        } else if (value <= 0xffff) {
            return 3;
        } else if (value <= 0xffffffff) {
            return 5;
        } else {
            return 9;
        }
    }
    uint64_t  get_value() { return value; }
    uint64_t  get_value() const { return value; }
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) {
        uint8_t vv = *(uint8_t *)p;
        if (vv < 0xfd) {
            value = *(uint8_t *)p;
            return 1;
        } else if (vv < 0xfe) {
            value = g_ntohs(*(uint16_t *)(p + 1));
            return 3;
        } else if (vv < 0xff) {
            value = g_ntohl(*(uint16_t *)(p + 1));
            return 5;
        } else {
            value = g_ntohll(*(uint16_t *)(p + 1));
            return 9;
        }
    }
    int output(char *p) {
        if (value < 0xfd) {
            *(uint8_t *)p = value;
            return 1;
        } else if (value <= 0xffff) {
            *(uint8_t *)p = 0xfd;
            *(uint16_t *)(p + 1) = g_htons(value);
            return 3;
        } else if (value <= 0xffffffff) {
            *(uint8_t *)p = 0xfe;
            *(uint32_t *)(p + 1) = g_htonl(value);
            return 5;
        } else {
            *(uint8_t *)p = 0xff;
            *(uint64_t *)(p + 1) = g_htonll(value);
            return 9;
        }
    }

    var_int_type &operator = (uint64_t v) { value = v; return *this; }

};

struct var_char_type {
    var_int_type    len;
    char            *value;

    var_char_type() : value(NULL) {}
    ~var_char_type() {
        if (value) {
            free(value);

        }
    }
    int  size() { return len.get_value() + len.size(); }
    const char *get_value() { return value; }
    const char *get_value() const { return value; }
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) {
        int last_len = len.get_value();
        int offset = len.init(p);
        if (value && last_len < len.get_value()) {
            free(value);
            value = NULL;
        }
        if (value == NULL) {
            value = (char *)malloc(len.get_value());
        }
        memcpy(value, p + offset, len.get_value());
        *(value + len.get_value()) = 0;
        return len.get_value() + len.size();
    }
    int output(char *p) {
        int offset = len.output(p);
        memcpy(p + offset, value, len.get_value());
        return offset + len.get_value();
    }

    var_char_type &operator = (const char *v) {
        int l = strlen(v);
        if (value && len.get_value() < l) {
            free(value);
            value = NULL;
        }
        if (value == NULL) {
            value = (char *)malloc(l);
        }
        len = l;
        memcpy(value, v, l);
        return *this;
    }

    std::string to_string() const {
        if (value == NULL) {
            return "";
        }
        return std::string(value, len.get_value());
    }
};

struct addr_type {
    uint64_type         service;
    fix_char_type<16>   ip;
    uint16_type         port;

    int size() { return service.size() + ip.size() + port.size(); }
    const char *get_value() { return ""; }
    const char *get_value() const { return ""; }
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) {
        int offset = service.init(p);
        offset += ip.init(p + offset);
        offset += port.init(p + offset);
        return size();
    }
    int output(char *p) {
        int offset = service.output(p);
        offset += ip.output(p + offset);
        offset += port.output(p + offset);
        return offset;
    }
    addr_type &operator =(const bcus::endpoint &ep) {
        service = 1;
        uint32_t ip_i = ep.ip_data();
        uint8_t *p = (uint8_t *)&ip_i;
        ip[12] = *(p + 0);
        ip[13] = *(p + 1);
        ip[14] = *(p + 2);
        ip[15] = *(p + 3);
        port = ep.port();
        return *this;
    }

    std::string to_string() const {
        return string_helper::format("%d.%d.%d.%d:%d", ip[12], ip[13], ip[14], ip[15], port);
    }
};



template<uint32_t BITS>
struct base_blob_int {
    enum { BYTE_NUM = BITS / 8 };
public:
    base_blob_int() {
        memset(value, 0, sizeof(value));
    }

    base_blob_int(uint64_t v)
    {
        value[0] = (uint32_t)v;
        value[1] = (uint32_t)(v >> 32);
        for (int i = 2; i < BYTE_NUM; i++)
            value[i] = 0;
    }

    int size() { return BYTE_NUM; }
    const char *get_value() { return (const char *)(value); }
    const char *get_value() const { return (const char *)(value); }
    int init(const char *p, int len) { return init(p); }
    int init(const char *p) {
        memcpy(value, p, BYTE_NUM);
        return BYTE_NUM;
    }
    int output(char *p) {
        memcpy(p, value, BYTE_NUM);
        return BYTE_NUM;
    }

    std::string to_string() const {
        char sz[BYTE_NUM * 2];
        cipher::bin_to_hex(value, BYTE_NUM, sz);
        return std::string(sz, BYTE_NUM * 2);
    }

    base_blob_int& operator = (uint64_t v) {
        memset(value, 0, sizeof(value));
        value[0] = (uint32_t)v;
        value[1] = (uint32_t)(v >> 32);
        return *this;
    }

    base_blob_int& operator++() { // prefix operator
        int i = 0;
        while (i < BYTE_NUM && ++value[i] == 0)
            i++;
        return *this;
    }

    const base_blob_int operator++(int) { // postfix operator
        const base_blob_int ret = *this;
        ++(*this);
        return ret;
    }

    bool empty() const {
        for (int i = 0; i < BYTE_NUM; i++)
            if (value[i] != 0)
                return false;
        return true;
    }

    inline int compare(const base_blob_int& other) const { return memcmp(value, other.value, sizeof(value)); }

    friend inline bool operator==(const base_blob_int& a, const base_blob_int& b) { return a.compare(b) == 0; }
    friend inline bool operator!=(const base_blob_int& a, const base_blob_int& b) { return a.compare(b) != 0; }
    friend inline bool operator<(const base_blob_int& a, const base_blob_int& b) { return a.compare(b) < 0; }


protected:
    uint8_t value[BYTE_NUM];
};

typedef base_blob_int<160> uint160_type;
typedef base_blob_int<256> uint256_type;

}

#endif
