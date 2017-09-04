#include "cc_parser.h"
#include "loghelper.h"
#include "cipher.h"

namespace bcus {

cc_decoder::cc_decoder() {
}
int cc_decoder::decode (const char *buf, int len) {
    buf_ = buf;
    if (len < sizeof(cc_header)) {
        return 0;
    }
    cc_header *h = (cc_header *)buf;
    uint32_t bodylen = g_ntohl(h->bodylen);
    if (len < sizeof(cc_header) + bodylen) {
        return 0;
    }
    len_ = sizeof(cc_header) + bodylen;
    /** todo caculate checksum */
    return len_;
}

uint16_t    cc_decoder::get_net() const {
    return g_ntohs(((const cc_header *)buf_)->net);
}
uint16_t    cc_decoder::get_version() const {
    return g_ntohs(((const cc_header *)buf_)->version);
}
uint16_t    cc_decoder::get_command() const {
    return g_ntohs(((const cc_header *)buf_)->command);
}
const char *cc_decoder::get_body() const {
    return buf_ + sizeof(cc_header);
}
uint16_t   cc_decoder::get_bodylen() const  {
    return g_ntohs(((const cc_header *)buf_)->bodylen);
}

std::string cc_decoder::to_string(int indent) const {
    std::string str(indent * 2, ' ');

    cc_header *h = (cc_header *)buf_;
    char sz[512] = {0};
    snprintf(sz, sizeof(sz) - 1, "net:0x%08x, version:%d, command:0X%04X, bodylen: %d, checksum:%u",
        g_ntohs(h->net), g_ntohs(h->version), g_ntohs(h->command), g_ntohs(h->bodylen), g_ntohl(h->checksum));
    str.append(sz);
    return str;
}
void cc_decoder::dump() const {
    XLOG(XLOG_DEBUG, "%s\n", to_string().c_str());
}


void  cc_encoder::encode() {
    uint8_t *body = (uint8_t *)(buffer_.base() + sizeof(cc_header));
    int bodylen = buffer_.len() - sizeof(cc_header);

    cc_header *h = (cc_header *)(buffer_.base());
    h->bodylen = g_htonl(bodylen);

    if (bodylen <= 0) {
        h->checksum = g_htonl(0xe0e2f65d);
        return;
    }

    uint8_t out1[32] = {0};
    uint8_t out2[32] = {0};
    bcus::cipher::sha256((const uint8_t *)body, bodylen, out1);
    bcus::cipher::sha256(out1, sizeof(out1), out2);

    h->checksum = g_htonl(*(uint32_t *)out2);
}
void cc_encoder::init(uint16_t net, uint16_t command) {
    buffer_.reset_loc(0);
    cc_header *h   = (cc_header *)(buffer_.base());
    h->net       = g_htons(net);
    h->command   = g_htons(command);
    h->version   = g_htons(BCUS__CC_VERSION);
    h->bodylen   = 0;
    h->checksum  = 0;

    buffer_.reset_loc(sizeof(cc_header));
}
void cc_encoder::body(const char *body, int len) {
    buffer_.append(body, len);
}

}
