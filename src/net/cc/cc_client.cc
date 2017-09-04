#include "cc_client.h"
#include "cc_command.h"

namespace bcus {

cc_client::cc_client(boost::asio::io_service &io_service) :
    io_service_(io_service), connector_(io_service_)
{
    connector_.set_create_socket_func(boost::bind(&cc_client::create_socket, this));
    connector_.set_callback(boost::bind(&cc_client::connect_result, this, _1, _2, _3, _4));
}
void cc_client::register_addr(const char *addr, int port)
{
    io_service_.post(boost::bind(&cc_client::do_connect, this, std::string(addr), port));
}

void cc_client::do_connect(const std::string &addr, int port)
{
    connector_.connect(addr.c_str(), port, 3);
}

void cc_client::connect_result(tcp_socket_ptr &socket,
                        const bcus::endpoint &ep,
                        const bcus::error_code &err,
                        void *context)
{
    if (err) {
        XLOG(XLOG_WARNING, "cc_client::%s, connected failed, ep[%s], err[%d]\n", __FUNCTION__, ep.to_string().c_str(), err);
        return;
    }

    cc_session *session = new cc_session(io_service_, socket);
    session->set_read_callback(boost::bind(&cc_client::on_read, this, _1, _2, _3));
    session->set_close_callback(boost::bind(&cc_client::on_peer_close, this, _1, _2));
    map_session_[session->session_id()] = session;

    connect_callback_(session->session_id(), ep);
}

void cc_client::on_read(  bcus::cc_session *session,
               cc_decoder *d,
               const bcus::endpoint &ep)
{
    //XLOG(XLOG_DEBUG, "cc_client::%s, ep[%s], %s\n", __FUNCTION__, ep.to_string().c_str(), d->to_string().c_str());
    read_callback_(session->session_id(), d, ep);
}

void cc_client::on_peer_close( bcus::cc_session *session, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "cc_client::%s, ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    SESSION_MAP::iterator itr = map_session_.find(session->session_id());
    if (itr == map_session_.end()) {
        XLOG(XLOG_WARNING, "cc_client::%s, reclose, ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
        return;
    }
    map_session_.erase(itr);
    delete session;
}

void cc_client::send_to_all(const void *buf, int len) {
    for(SESSION_MAP::iterator itr = map_session_.begin(); itr != map_session_.end(); ++itr) {
        itr->second->send(buf, len);
    }
}
void cc_client::send(uint32_t session_id, const void *buf, int len) {
    SESSION_MAP::iterator itr = map_session_.find(session_id);
    if (itr == map_session_.end()) {
        XLOG(XLOG_WARNING, "cc_client::%s, remote is not existed\n", __FUNCTION__);
        return;
    }
    itr->second->send(buf, len);
}

}

