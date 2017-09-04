#include "cc_session_container.h"
#include "loghelper.h"

namespace bcus {

cc_session_container::cc_session_container(uint32_t id, boost::asio::io_service &io_service) :
    id_(id), io_service_(io_service)
{
}

cc_session_container::~cc_session_container() {
    typename SesstionMap::iterator itr = map_session_.begin();
    for (; itr != map_session_.end(); ++itr) {
        delete itr->second;
    }
    map_session_.clear();
}

void cc_session_container::do_accepted(tcp_socket_ptr &socket)
{
    XLOG(XLOG_DEBUG, "cc_session_container::%s, fd[%u]\n", __FUNCTION__, socket->native_handle());

    cc_session *session = new bcus::cc_session(io_service_, socket);
    session->set_read_callback(boost::bind(&cc_session_container::on_read, this, _1, _2, _3));
    session->set_close_callback(boost::bind(&cc_session_container::on_peer_close, this, _1, _2));

    map_session_[session->session_id()] = session;
}

void cc_session_container::on_peer_close(cc_session *session, const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "cc_session_container::%s, session_id[%u], addr[%s]\n", __FUNCTION__, session->session_id(), ep.to_string().c_str());
    SesstionMap::iterator itr = map_session_.find(session->session_id());
    if (itr != map_session_.end()) {
        delete itr->second;
        map_session_.erase(itr);
    }
}

void cc_session_container::on_read(cc_session *session, cc_decoder *d, const bcus::endpoint &ep)
{
    //XLOG(XLOG_DEBUG, "cc_session_container::%s, session[%llu], addr[%s:%u], ec[%d]\n", __FUNCTION__,
    //    session->session_id(), ip, port, err);

    callback_(id_, session->session_id(), d, ep);
}
int cc_session_container::send_to_all(const void *buf, int len) {
    SesstionMap::iterator itr = map_session_.begin();
    for (; itr != map_session_.end(); ++itr) {
        itr->second->send(buf, len);
    }
    return 0;
}
int cc_session_container::send(uint32_t sessionid, const void *buf, int len) {
    SesstionMap::iterator itr = map_session_.find(sessionid);
    if (itr == map_session_.end()) {
        XLOG(XLOG_WARNING, "cc_session_container::%s, session_id[%u], is not exist\n", __FUNCTION__, sessionid);
        return -1;;
    }
    itr->second->send(buf, len);
    return 0;
}

}


