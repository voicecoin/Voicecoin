#include "http_session_container.h"
#include "loghelper.h"

namespace bcus {

http_session_container::http_session_container(uint32_t id, boost::asio::io_service &io_service) :
    id_(id), io_service_(io_service)
{
}

http_session_container::~http_session_container() {
    typename SesstionMap::iterator itr = map_session_.begin();
    for (; itr != map_session_.end(); ++itr) {
        delete itr->second;
    }
    map_session_.clear();
}

void http_session_container::do_accepted(tcp_socket_ptr &socket)
{
    XLOG(XLOG_DEBUG, "http_session_container::%s, fd[%u]\n", __FUNCTION__, socket->native_handle());

    http_session *session = new bcus::http_session(io_service_, socket);
    session->set_read_callback(boost::bind(&http_session_container::on_read, this, _1, _2, _3, _4));
    session->set_close_callback(boost::bind(&http_session_container::on_peer_close, this, _1, _2));

    map_session_[session->session_id()] = session;
}

void http_session_container::on_peer_close(http_session *session, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "http_session_container::%s, session_id[%u], addr[%s]\n", __FUNCTION__, session->session_id(), ep.to_string().c_str());
    typename SesstionMap::iterator itr = map_session_.find(session->session_id());
    if (itr != map_session_.end()) {
        delete itr->second;
        map_session_.erase(itr);
    }
}

void http_session_container::on_read(http_session *session, http_decoder *d, const bcus::endpoint &ep, void *)
{
    //XLOG(XLOG_DEBUG, "http_session_container::%s, session[%llu], addr[%s:%u], ec[%d]\n", __FUNCTION__,
    //    session->session_id(), ip, port, err);

    callback_(id_, session->session_id(), d, ep);
}

int http_session_container::send_response(uint32_t sessionid, uint32_t seqno, const void *buf, int len) {
    typename SesstionMap::iterator itr = map_session_.find(sessionid);
    if (itr == map_session_.end()) {
        XLOG(XLOG_WARNING, "http_session_container::%s, session_id[%u], is not exist\n", __FUNCTION__, sessionid);
        return -1;;
    }
    itr->second->do_response(seqno, buf, len);
    return 0;
}

}


