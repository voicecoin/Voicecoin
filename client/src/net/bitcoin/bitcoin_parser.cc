#include "bitcoin_parser.h"
#include "loghelper.h"
#include "cipher.h"

namespace bcus {

bitcoin_decoder::bitcoin_decoder() {
}
int bitcoin_decoder::decode (const char *buf, int len) {
    buf_ = buf;
    if (len < sizeof(bitcoin_head)) {
        return 0;
    }
    bitcoin_head *h = (bitcoin_head *)buf;
    uint32_t body_len = g_ntohl(h->body_len);
    if (len < sizeof(bitcoin_head) + body_len) {
        return 0;
    }
    len_ = sizeof(bitcoin_head) + body_len;
    return len_;
}

const uint32_t bitcoin_decoder::type() const {
    bitcoin_head *h = (bitcoin_head *)buf_;
    return g_ntohl(h->type);
}
const bcus::slice bitcoin_decoder::command() const {
    bitcoin_head *h = (bitcoin_head *)buf_;
    int len = strlen((const char *)(h->command));
    if (len > sizeof(h->command)) {
        len = sizeof(h->command);
    }
    return slice((const char *)(h->command), len);
}
const bcus::slice bitcoin_decoder::body() const {
    return slice(buf_ + sizeof(bitcoin_head), len_ - sizeof(bitcoin_head));
}
std::string bitcoin_decoder::to_string(int indent) const {
    std::string str(indent * 2, ' ');

    bitcoin_head *h = (bitcoin_head *)buf_;
    char sz[512] = {0};
    snprintf(sz, sizeof(sz) - 1, "type:0x%08x, command:%s, body_len:%u, check_sum:%u",
        g_ntohl(h->type), std::string((const char *)(h->command), sizeof(h->command)).c_str(),
        g_ntohl(h->body_len), g_ntohl(h->check_sum));
    str.append(sz);
    return str;
}
void bitcoin_decoder::dump() const {
    XLOG(XLOG_DEBUG, "%s\n", to_string().c_str());
}


void  bitcoin_encoder::encode() {
    uint8_t *body = (uint8_t *)(buffer_.base() + sizeof(bitcoin_head));
    int body_len = buffer_.len() - sizeof(bitcoin_head);

    bitcoin_head *h = (bitcoin_head *)(buffer_.base());
    h->body_len = g_htonl(body_len);

    if (body_len <= 0) {
        h->check_sum = g_htonl(0xe0e2f65d);
        return;
    }

    uint8_t out1[32] = {0};
    uint8_t out2[32] = {0};
    bcus::cipher::sha256((const uint8_t *)body, body_len, out1);
    bcus::cipher::sha256(out1, sizeof(out1), out2);

    h->check_sum = g_htonl(*(uint32_t *)out2);
}
void bitcoin_encoder::init(uint32_t type, const char *command) {
    buffer_.reset_loc(0);
    bitcoin_head *h = (bitcoin_head *)(buffer_.base());
    h->type = g_htonl(type);

    int command_len = strlen(command);
    if (command_len > sizeof(h->command))
        command_len = sizeof(h->command);
    memcpy(h->command, command, command_len);

    buffer_.reset_loc(sizeof(bitcoin_head));
}
void bitcoin_encoder::body(const char *body, int len) {
    bitcoin_head *h = (bitcoin_head *)(buffer_.base());
    h->body_len = g_htonl(len);
    buffer_.append(body, len);
    /** todo caculate check sum */
}

}
