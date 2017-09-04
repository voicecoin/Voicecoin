#include "bitcoin_client.h"
#include "bitcoin_message.h"

namespace bcus {

bitcoin_client::bitcoin_client(boost::asio::io_service &io_service) :
    io_service_(io_service), connector_(io_service_)
{
    connector_.set_create_socket_func(boost::bind(&bitcoin_client::create_socket, this));
    connector_.set_callback(boost::bind(&bitcoin_client::connect_result, this, _1, _2, _3, _4));
}
void bitcoin_client::register_addr(const char *addr, int port)
{
    io_service_.post(boost::bind(&bitcoin_client::do_connect, this, std::string(addr), port));
}

void bitcoin_client::do_connect(const std::string &addr, int port)
{
    connector_.connect(addr.c_str(), port, 3);
}

void bitcoin_client::connect_result(tcp_socket_ptr &socket,
                        const bcus::endpoint &ep,
                        const bcus::error_code &err,
                        void *context)
{
    if (err) {
        XLOG(XLOG_WARNING, "bitcoin_client::%s, connected failed, ep[%s], err[%d]\n", __FUNCTION__, ep.to_string().c_str(), err);
        return;
    }

    bitcoin_session *session = new bitcoin_session(io_service_, socket);
    session->set_read_callback(boost::bind(&bitcoin_client::on_read, this, _1, _2, _3));
    session->set_close_callback(boost::bind(&bitcoin_client::on_peer_close, this, _1, _2));
    map_session_[session->session_id()] = session;

    do_send_version(session, ep);
}

void bitcoin_client::on_read(  bcus::bitcoin_session *session,
               bitcoin_decoder *d,
               const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "bitcoin_client::%s, ep[%s], %s\n", __FUNCTION__, ep.to_string().c_str(), d->to_string().c_str());
    //callback_(d);
    bcus::slice command = d->command();
    if (command == "version") {
        do_recv_version(session, d, ep);
    } else if (command == "verack") {
        do_recv_verback(session, d, ep);
    }
}

void bitcoin_client::on_peer_close( bcus::bitcoin_session *session, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "bitcoin_client::%s, ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    SESSION_MAP::iterator itr = map_session_.find(session->session_id());
    if (itr == map_session_.end()) {
        XLOG(XLOG_WARNING, "bitcoin_client::%s, reclose, ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
        return;
    }
    map_session_.erase(itr);
    delete session;
}


/**************************************************************************************************/
/***************************   send package function **********************************************/
/**************************************************************************************************/
void bitcoin_client::do_send_version(bitcoin_session *session, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "bitcoin_client::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    bitcoin_message_factory::create_version_msg(&en_, ep, 0, 0);
    session->send(en_.get_buf(), en_.get_len());
}
void bitcoin_client::do_send_verback(bitcoin_session *session, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "bitcoin_client::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    bitcoin_message_factory::create_verback_msg(&en_);
    session->send(en_.get_buf(), en_.get_len());
}



/**************************************************************************************************/
/***************************   recv package function **********************************************/
/**************************************************************************************************/
void bitcoin_client::do_recv_version(bitcoin_session *session, bitcoin_decoder *d, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "bitcoin_client::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    d->dump();

    bcus::slice body = d->body();
    bcus::version_msg msg;
    msg.init_from_stream(body.c_str(), body.length());
    msg.dump();
}
void bitcoin_client::do_recv_verback(bitcoin_session *session, bitcoin_decoder *d, const bcus::endpoint &ep) {
    XLOG(XLOG_DEBUG, "bitcoin_client::%s ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    d->dump();
}

}

