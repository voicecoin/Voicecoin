#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "http_server.h"
#include "loghelper.h"

namespace bcus {

http_server::http_server(boost::asio::io_service &io_service) :
    current_conatiner_(0)
{
    http_session_container *container = new http_session_container(0, io_service);
    vec_session_container_.push_back(container);

    init_acceptor(io_service);
}


http_server::http_server(std::vector<boost::asio::io_service *> vec_io_service) :
    current_conatiner_(0)
{
    if (vec_io_service.empty()) {
        throw std::runtime_error("http_server::http_server, vec_io_service is empty");
    }

    for (uint32_t index = 0; index < vec_io_service.size(); ++index) {
        http_session_container *container = new http_session_container(index, *(vec_io_service[index]));
        vec_session_container_.push_back(container);
    }

    init_acceptor(*vec_io_service[0]);
}


http_server::~http_server()
{
    delete acceptor_;
}

tcp_socket_ptr http_server::create_socket() {
    return tcp_socket_ptr(new tcp_socket(get_container()->get_io_service()));
}
void http_server::init_acceptor(boost::asio::io_service &io_service) {
    acceptor_ = new stream_acceptor(io_service);

    acceptor_->set_create_socket_func(boost::bind(&http_server::create_socket, this));
    acceptor_->set_callback(boost::bind(&http_server::on_accepted, this, _1));
}

void http_server::do_set_read_callback(READ_CALLBACK cb) {
    typename std::vector<http_session_container *>::iterator i = vec_session_container_.begin();
    for (; i != vec_session_container_.end(); ++i) {
        (*i)->set_callback(cb);
    }
}

void http_server::do_set_read_callback(uint32_t id, READ_CALLBACK cb) {
    if (id >= vec_session_container_.size()) {
        throw std::runtime_error("http_server::do_set_read_callback, bad id");
    }
    vec_session_container_[id]->set_callback(cb);
}

int  http_server::register_addr(const bcus::endpoint &ep) {
    return acceptor_->start(ep);
}

//void http_server::on_accepted(tcp_socket_ptr& socket)
//{
//    // 获取 io_context
//    auto& io_context = boost::asio::use_service<boost::asio::io_context>(socket->get_executor().context());
//
//    // 使用全局的 boost::asio::post 方法
//    boost::asio::post(io_context,
//        boost::bind(&http_session_container::do_accepted,
//            get_container(io_context), socket));
//}

void http_server::on_accepted(tcp_socket_ptr& socket)
{
    // 获取与 socket 相关的 io_context。确保执行器实际上是 io_context 的一部分。
    auto& io_context = static_cast<boost::asio::io_context&>(socket->get_executor().context());

    // 使用全局的 boost::asio::post 方法
    boost::asio::post(io_context.get_executor(),
        [this, &socket, &io_context]() {  // 使用 & 来捕获 socket 和 io_context 引用
            auto container = get_container(io_context);
            container->do_accepted(socket);  // 现在传递的是正确的引用
        });
}


int  http_server::send_response(uint32_t containerid, uint32_t sessionid, uint32_t seqno, const void *buf, int len) {
    vec_session_container_[containerid]->send_response(sessionid, seqno, buf, len);
    return 0;
}

http_session_container *http_server::get_container() {
    return vec_session_container_[current_conatiner_++ % vec_session_container_.size()];
}
http_session_container *http_server::get_container(const boost::asio::io_service &io_service) {
    std::vector<http_session_container *>::iterator itr = vec_session_container_.begin();
    for (; itr != vec_session_container_.end(); ++itr) {
        const boost::asio::io_service &dst_io_service = (*itr)->get_io_service();
        if (&dst_io_service == &io_service) {
            return *itr;
        }
    }
    return NULL;
}


}

