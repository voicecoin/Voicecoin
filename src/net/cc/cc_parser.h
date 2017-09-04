#ifndef CC_PARSER_H
#define CC_PARSER_H

#include "parser.h"
#include "slice.h"
#include "cc_command.h"
#include <map>

namespace bcus {

#define BCUS__CC_VERSION 1

enum {
    MAINNET    = 0xbef9,
    TESTNET    = 0x110b,
    REGTEST    = 0xbffa
};

struct cc_header {
    uint16_t net;
    uint16_t version;
    uint16_t command;
    uint16_t bodylen;
    uint32_t checksum;
};


class cc_decoder : public bcus::decoder {
public:
    cc_decoder();
    virtual ~cc_decoder() { }
    virtual int decode(const char *buf, int len);

    uint16_t    get_net() const;
    uint16_t    get_version() const;
    uint16_t    get_command() const;
    const char *get_body() const;
    uint16_t    get_bodylen() const;

    std::string to_string(int indent = 0) const;
    void dump() const;
};

class cc_encoder : public bcus::encoder {
public:
    virtual ~cc_encoder() {}

    virtual void        encode();

    void init(uint16_t net, uint16_t command);
    void body(const char *body, int len);
};

}

#endif
