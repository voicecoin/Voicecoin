#include "bitcoin_message.h"
#include "loghelper.h"

namespace bcus {

int version_msg::SMALL_LEN() {
    return  version.size() +
            services.size() +
            timestamp.size() +
            addr_recv.size();
}
int version_msg::init_from_stream(const char *p, int len) {
    if (len < SMALL_LEN()) {
        return -1;
    }
    int offset = 0;
    offset += version.init(p + offset);
    offset += services.init(p + offset);
    offset += timestamp.init(p + offset);
    offset += addr_recv.init(p + offset);

    if (version.get_value() < 106) { return offset; }
    offset += addr_from.init(p + offset);
    offset += nodeid.init(p + offset);
    offset += user_agent.init(p + offset);

    if (version.get_value() < 70001) { return offset; }
    offset += latest_blockid.init(p + offset);
    return offset;
}
int version_msg::output_to_stream(char *p) {
    int offset = 0;
    offset += version.output(p + offset);
    offset += services.output(p + offset);
    offset += timestamp.output(p + offset);
    offset += addr_recv.output(p + offset);

    if (version.get_value() < 106) { return offset; }
    offset += addr_from.output(p + offset);
    offset += nodeid.output(p + offset);
    offset += user_agent.output(p + offset);

    if (version.get_value() < 70001) { return offset; }
    offset += latest_blockid.output(p + offset);
    return offset;
}
void version_msg::dump() const {
    XLOG(XLOG_DEBUG, "version_msg:\n");
    XLOG(XLOG_DEBUG, "    version[%u]\n", version.get_value());
    XLOG(XLOG_DEBUG, "    services[%llu]\n", services.get_value());
    XLOG(XLOG_DEBUG, "    timestamp[%s]\n", timestamp.to_string().c_str());
    XLOG(XLOG_DEBUG, "    addr_recv[%s]\n", addr_recv.to_string().c_str());
    XLOG(XLOG_DEBUG, "    addr_from[%s]\n", addr_from.to_string().c_str());
    XLOG(XLOG_DEBUG, "    nodeid[%llu]\n", nodeid.get_value());
    XLOG(XLOG_DEBUG, "    user_agent[%s]\n", user_agent.to_string().c_str());
    XLOG(XLOG_DEBUG, "    latest_blockid[%u]\n", latest_blockid.get_value());
}

/***************************************************************************/
/***************************************************************************/
int ping_msg::init_from_stream(const char *p, int len) {
    int offset = 0;
    offset += random.init(p + offset);
    return offset;
}
int ping_msg::output_to_stream(char *p) {
    int offset = 0;
    offset += random.output(p + offset);
    return offset;
}
void ping_msg::dump() const {
    XLOG(XLOG_DEBUG, "ping_msg:\n");
    XLOG(XLOG_DEBUG, "    random[%llu]\n", random.get_value());
}
/***************************************************************************/
/***************************************************************************/
int pong_msg::init_from_stream(const char *p, int len) {
    int offset = 0;
    offset += random.init(p + offset);
    return offset;
}
int pong_msg::output_to_stream(char *p) {
    int offset = 0;
    offset += random.output(p + offset);
    return offset;
}
void pong_msg::dump() const {
    XLOG(XLOG_DEBUG, "pong_msg:\n");
    XLOG(XLOG_DEBUG, "    random[%llu]\n", random.get_value());
}



void bitcoin_message_factory::create_version_msg(bitcoin_encoder *en,
    const bcus::endpoint &ep, uint64_t nodeid, uint32_t blockid) {
    en->init(MESSAGE_MAIN, "version");

    version_msg msg;
    msg.version = 70015;
    msg.services = 1;
    msg.timestamp = time(NULL);
    msg.addr_recv = ep;
    //msg.addr_from;
    msg.nodeid = nodeid;
    msg.user_agent = "testtttt";
    msg.latest_blockid = blockid;

    int len = msg.output_to_stream(en->get_buffer()->top());
    en->get_buffer()->inc_loc(len);
    en->encode();
}
void bitcoin_message_factory::create_verback_msg(bitcoin_encoder *en) {
    en->init(MESSAGE_MAIN, "verback");
    en->encode();
}
void bitcoin_message_factory::create_getaddr_msg(bitcoin_encoder *en) {
    en->init(MESSAGE_MAIN, "getaddr");
    en->encode();
}
void bitcoin_message_factory::create_sendheaders_msg(bitcoin_encoder *en) {
    en->init(MESSAGE_MAIN, "sendheaders");
    en->encode();
}
void bitcoin_message_factory::create_sendcmpct_msg(bitcoin_encoder *en) {
    en->init(MESSAGE_MAIN, "sendcmpct");
    /** todo set body */
    en->encode();
}
void bitcoin_message_factory::create_ping_msg(bitcoin_encoder *en) {
    en->init(MESSAGE_MAIN, "ping");
    ping_msg msg;
    msg.random = time(NULL);
    int len = msg.output_to_stream(en->get_buffer()->top());
    en->get_buffer()->inc_loc(len);
    en->encode();
}
void bitcoin_message_factory::create_pong_msg(bitcoin_encoder *en, uint64_t random) {
    en->init(MESSAGE_MAIN, "pong");
    pong_msg msg;
    msg.random = random;
    int len = msg.output_to_stream(en->get_buffer()->top());
    en->get_buffer()->inc_loc(len);
    en->encode();
}


}

