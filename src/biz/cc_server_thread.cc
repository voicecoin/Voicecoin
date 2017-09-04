#include "cc_server_thread.h"


namespace bcus {

cc_server_thread::cc_server_thread() : server_(io_service_) {
    server_.set_read_callback(boost::bind(&cc_server_thread::on_read, this, _1, _2, _3, _4));
}

void cc_server_thread::start() {
    thread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
}
void cc_server_thread::join() {
    thread_.join();
}
void cc_server_thread::start_server(const char *ip, int port) {
    server_.register_addr(bcus::endpoint(ip, port));
}
void cc_server_thread::on_read(  uint32_t containerid, uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "cc_server_thread::%s, ep[%s], %s\n", __FUNCTION__, ep.to_string().c_str(), d->to_string().c_str());
    switch(d->get_command()) {
      case CC_VERSION: { do_recv_version(session_id, d, ep); break; }
      case CC_VERBACK: { do_recv_verback(session_id, d, ep); break; }
      default: { callback_(session_id, d, ep); break; }
    }
}
void cc_server_thread::send_to_all(const void *buf, int len)
{
    server_.send_to_all(buf, len);
}
void cc_server_thread::send(uint32_t session_id, const void *buf, int len) {
    server_.send(0, session_id, buf, len);
}

/**************************************************************************************************/
/***************************   send package function **********************************************/
/**************************************************************************************************/
void cc_server_thread::do_send_version(uint32_t session_id, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_server_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    en_.init(MAINNET, CC_VERSION);
    server_.send(0, session_id, en_.get_buf(), en_.get_len());
}
void cc_server_thread::do_send_verback(uint32_t session_id, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_server_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    en_.init(MAINNET, CC_VERBACK);
    server_.send(0, session_id, en_.get_buf(), en_.get_len());
}

/**************************************************************************************************/
/***************************   recv package function **********************************************/
/**************************************************************************************************/
void cc_server_thread::do_recv_version(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_server_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    d->dump();
    do_send_verback(session_id, ep);
    do_send_version(session_id, ep);
}
void cc_server_thread::do_recv_verback(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_server_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    d->dump();
}

}



