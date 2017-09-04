#include "cc_server.h"
#include "loghelper.h"

namespace bcus {

cc_server::cc_server(boost::asio::io_service &io_service) :
    current_conatiner_(0)
{
    cc_session_container *container = new cc_session_container(0, io_service);
    vec_session_container_.push_back(container);

    init_acceptor(io_service);
}


cc_server::cc_server(std::vector<boost::asio::io_service *> vec_io_service) :
    current_conatiner_(0)
{
    if (vec_io_service.empty()) {
        throw std::runtime_error("cc_server::cc_server, vec_io_service is empty");
    }

    for (uint32_t index = 0; index < vec_io_service.size(); ++index) {
        cc_session_container *container = new cc_session_container(index, *(vec_io_service[index]));
        vec_session_container_.push_back(container);
    }

    init_acceptor(*vec_io_service[0]);
}


cc_server::~cc_server()
{
    delete acceptor_;
}

tcp_socket_ptr cc_server::create_socket() {
    return tcp_socket_ptr(new tcp_socket(get_container()->get_io_service()));
}
void cc_server::init_acceptor(boost::asio::io_service &io_service) {
    acceptor_ = new stream_acceptor(io_service);

    acceptor_->set_create_socket_func(boost::bind(&cc_server::create_socket, this));
    acceptor_->set_callback(boost::bind(&cc_server::on_accepted, this, _1));
}

void cc_server::do_set_read_callback(READ_CALLBACK cb) {
    typename std::vector<cc_session_container *>::iterator i = vec_session_container_.begin();
    for (; i != vec_session_container_.end(); ++i) {
        (*i)->set_callback(cb);
    }
}

void cc_server::do_set_read_callback(uint32_t id, READ_CALLBACK cb) {
    if (id >= vec_session_container_.size()) {
        throw std::runtime_error("cc_server::do_set_read_callback, bad id");
    }
    vec_session_container_[id]->set_callback(cb);
}

int  cc_server::register_addr(const bcus::endpoint &ep) {
    return acceptor_->start(ep);
}

void cc_server::on_accepted(tcp_socket_ptr &socket)
{
    //XLOG(XLOG_DEBUG, "fib::responser::%s, type[0X%04X], fd[%d], ep[%s]\n", __FUNCTION__, type, fd, target.to_string().c_str());

    socket->get_io_service().post(boost::bind(&cc_session_container::do_accepted, get_container(socket->get_io_service()), socket));
}
int cc_server::send_to_all(const void *buf, int len) {
    std::vector<cc_session_container *>::iterator itr = vec_session_container_.begin();
    for (; itr != vec_session_container_.end(); ++itr) {
        (*itr)->send_to_all(buf, len);
    }
    return 0;
}

int  cc_server::send(uint32_t containerid, uint32_t sessionid, const void *buf, int len) {
    vec_session_container_[containerid]->send(sessionid, buf, len);
    return 0;
}

cc_session_container *cc_server::get_container() {
    return vec_session_container_[current_conatiner_++ % vec_session_container_.size()];
}
cc_session_container *cc_server::get_container(const boost::asio::io_service &io_service) {
    std::vector<cc_session_container *>::iterator itr = vec_session_container_.begin();
    for (; itr != vec_session_container_.end(); ++itr) {
        const boost::asio::io_service &dst_io_service = (*itr)->get_io_service();
        if (&dst_io_service == &io_service) {
            return *itr;
        }
    }
    return NULL;
}


}

