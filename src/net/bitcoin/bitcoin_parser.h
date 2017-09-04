#ifndef BITCOIN_PARSER_H
#define BITCOIN_PARSER_H

#include "parser.h"
#include "slice.h"
#include <map>

namespace bcus {

enum {
    MESSAGE_MAIN    = 0xd9b4bef9,
    MESSAGE_TESTNET = 0x0709110b,
    MESSAGE_REGTEST = 0xdab6bffa
};

struct bitcoin_head {
    uint32_t type;
    uint8_t  command[12];
    uint32_t body_len;
    uint32_t check_sum;
};

class bitcoin_decoder : public bcus::decoder {
public:
    bitcoin_decoder();
    virtual ~bitcoin_decoder() { }
    virtual int decode(const char *buf, int len);

    const uint32_t type() const;
    const bcus::slice command() const;
    //const uint32_t check_sum() const;
    const bcus::slice body() const;

    std::string to_string(int indent = 0) const;
    void dump() const;
};

class bitcoin_encoder : public bcus::encoder {
public:
    virtual ~bitcoin_encoder() {}

    virtual void        encode();

    void init(uint32_t type, const char *command);
    void body(const char *body, int len);
};

}

#endif
