#ifndef BITCOIN_MESSAGE_H
#define BITCOIN_MESSAGE_H

#include "bitcoin_type.h"
#include "bitcoin_parser.h"

namespace bcus {

/*
    4   version int32_t Identifies protocol version being used by the node
    8   services    uint64_t    bitfield of features to be enabled for this connection
    8   timestamp   int64_t standard UNIX timestamp in seconds
    26  addr_recv   net_addr    The network address of the node receiving this message

    Fields below require version ≥ 106
    26  addr_from   net_addr    The network address of the node emitting this message
    8   nonce   uint64_t    Node random nonce, randomly generated every time a version packet is sent.
                            This nonce is used to detect connections to self.
    ?  user_agent  var_str User Agent (0x00 if string is 0 bytes long)
    4   start_height    int32_t The last block received by the emitting node

    Fields below require version ≥ 70001
    1   relay   bool    Whether the remote peer should announce relayed transactions or not, see BIP 0037
*/
struct version_msg {
    uint32_type      version;
    uint64_type      services;
    time_type        timestamp;
    addr_type        addr_recv;
    addr_type        addr_from;
    uint64_type      nodeid;
    var_char_type    user_agent;
    uint32_type      latest_blockid;

    int  SMALL_LEN();
    int  init_from_stream(const char *p, int len);
    int  output_to_stream(char *p);
    void dump() const;
};

struct ping_msg {
    uint64_type random;
    int  init_from_stream(const char *p, int len);
    int  output_to_stream(char *p);
    void dump() const;
};
struct pong_msg {
    uint64_type random;   //read from ping
    int  init_from_stream(const char *p, int len);
    int  output_to_stream(char *p);
    void dump() const;
};

class bitcoin_message_factory {
public:
    static void create_version_msg(bitcoin_encoder *en, const bcus::endpoint &ep, uint64_t nodeid, uint32_t blockid);
    static void create_verback_msg(bitcoin_encoder *en);
    static void create_getaddr_msg(bitcoin_encoder *en);
    static void create_sendheaders_msg(bitcoin_encoder *en);
    static void create_sendcmpct_msg(bitcoin_encoder *en);
    static void create_ping_msg(bitcoin_encoder *en);
    static void create_pong_msg(bitcoin_encoder *en, uint64_t random);
};

}

#endif
