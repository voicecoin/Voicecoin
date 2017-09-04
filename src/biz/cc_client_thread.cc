#include "cc_client_thread.h"


namespace bcus {

cc_client_thread::cc_client_thread() : client_(io_service_) {
    client_.set_connected_callback(boost::bind(&cc_client_thread::on_connected, this, _1, _2));
    client_.set_read_callback(boost::bind(&cc_client_thread::on_read, this, _1, _2, _3));
}

void cc_client_thread::start() {
    thread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));
}
void cc_client_thread::join() {
    thread_.join();
}
void cc_client_thread::register_addr(const char *addr, int port) {
    client_.register_addr(addr, port);
}
void cc_client_thread::on_connected(uint32_t session_id, const bcus::endpoint &ep) {
    do_send_version(session_id, ep);
}
void cc_client_thread::on_read(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "cc_client_thread::%s, ep[%s], %s\n", __FUNCTION__, ep.to_string().c_str(), d->to_string().c_str());
    switch(d->get_command()) {
      case CC_VERSION: { do_recv_version(session_id, d, ep); break; }
      case CC_VERBACK: { do_recv_verback(session_id, d, ep); break; }
      default: { callback_(session_id, d, ep); break; }
    }
}
void cc_client_thread::send_to_all(const void *buf, int len)
{
    client_.send_to_all(buf, len);
}
void cc_client_thread::send(uint32_t session_id, const void *buf, int len) {
    client_.send(session_id, buf, len);
}

/**************************************************************************************************/
/***************************   send package function **********************************************/
/**************************************************************************************************/
void cc_client_thread::do_send_version(uint32_t session_id, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_client_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    en_.init(MAINNET, CC_VERSION);
    client_.send(session_id, en_.get_buf(), en_.get_len());
}
void cc_client_thread::do_send_verback(uint32_t session_id, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_client_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    en_.init(MAINNET, CC_VERBACK);
    client_.send(session_id, en_.get_buf(), en_.get_len());
}

/**************************************************************************************************/
/***************************   recv package function **********************************************/
/**************************************************************************************************/
void cc_client_thread::do_recv_version(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_client_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    d->dump();
    do_send_verback(session_id, ep);
}
void cc_client_thread::do_recv_verback(uint32_t session_id, cc_decoder *d, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "cc_client_thread::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    d->dump();
}

}



