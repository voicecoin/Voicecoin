#include "http_client.h"

namespace bcus {

struct http_client::session_info {
    struct req {
        std::string path;
        void *context;
        req() : context(NULL) { }
        req(const char *path, void *context) {
            this->path = path;
            this->context = context;
        }
    };

    std::string domain;
    bcus::http_session *session;
    std::deque<req> waiting_req_list;
    session_info(const char *domain) : session(NULL) {
        this->domain = domain;
    }
};

http_client::http_client(boost::asio::io_service &io_service) :
    io_service_(io_service), connector_(io_service_), req_seqno_(0)
{
    connector_.set_create_socket_func(boost::bind(&http_client::create_socket, this));
    connector_.set_callback(boost::bind(&http_client::connect_result, this,
            _1, _2, _3, _4));
}
int http_client::request(const char *url, void *context)
{
    char domain[128]={0};
    char path[256]={0};

    int ret = sscanf(url, "%*[HTTPhttp]://%[^/]%256s", domain, path);
    if ( 1 != ret && 2 != ret) {
        XLOG(XLOG_WARNING, "http_client::%s, bad url[%s]\n", __FUNCTION__, url);
        return -1;
    }
    if (1 == ret) {
        path[0] = '/';
    }

    session_info *info = NULL;
    SESSION_MAP::iterator itr = map_session_.find(domain);
    if (itr != map_session_.end()) {
        info = itr->second;
    } else {
        info = new session_info(domain);
        map_session_[domain] = info;
        do_connect(domain, info);
    }

    do_request(info, domain, path, context);

    return 0;
}
void http_client::do_connect(const char *domain, session_info *info)
{
    char host[128] = {0};
    int port = 80;
    sscanf(domain, "%[^:]:%d", host, &port);
    connector_.connect(host, port, 3, info);
}

void http_client::connect_result(tcp_socket_ptr &socket,
                        const bcus::endpoint &ep,
                        const bcus::error_code &err,
                        void *context)
{
    session_info *info = (session_info *)context;
    if (err) {
        reset(info);
        return;
    }

    info->session = new bcus::http_session(io_service_, socket);
    info->session->set_domain(info->domain.c_str());
    info->session->set_read_callback(boost::bind(&http_client::on_read, this, _1, _2, _3, _4));
    info->session->set_close_callback(boost::bind(&http_client::on_peer_close, this, _1, _2));

    while(! info->waiting_req_list.empty()) {
        session_info::req &r = info->waiting_req_list.front();
        do_request(info, info->domain.c_str(), r.path.c_str(), r.context);
        info->waiting_req_list.pop_front();
    }
}

void http_client::do_request(http_client::session_info *info,
    const char *domain, const char *path, void *context)
{
    if (info->session == NULL) {
         info->waiting_req_list.push_back(session_info::req(path, context));
         return;
    }

    XLOG(XLOG_DEBUG, "http_client::%s, domain[%s], path[%s], context[%p]\n",
        __FUNCTION__, domain, path, context);

    en_.init(domain, path, "GET");
    en_.head("Connection", "keep-alive");
    en_.head("Cache-Control", "max-age=0");
    en_.head("User-Agent", "Mozilla/5.0");
    en_.head("Accept", "text/html,application/xhtml+xml,application/xml;q=0.8");
    en_.head("Accept-Language", "en-US,en;q=0.8,zh-CN;q=0.6,zh-TW;q=0.4");
    en_.encode();

    //XLOG(XLOG_TRACE, "http_client::%s, %s\n",
    //    __FUNCTION__, std::string((const char *)(en_.get_buf()), en_.get_len()).c_str());
    info->session->do_request(req_seqno_++, en_.get_buf(), en_.get_len(), context);
}

void http_client::on_read(  bcus::http_session *session,
               http_decoder *d,
               const bcus::endpoint &ep,
               void *context)
{
    callback_(d, ep, context);
}

void http_client::on_peer_close( bcus::http_session *session,
                    const bcus::endpoint &ep)
{
    XLOG(XLOG_DEBUG, "http_client::%s, ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
    SESSION_MAP::iterator itr = map_session_.find(session->get_domain());
    if (itr == map_session_.end()) {
        XLOG(XLOG_WARNING, "http_client::%s, reclose, ep[%s]\n", __FUNCTION__, ep.to_string().c_str());
        return;
    }
    session_info *info = itr->second;
    reset(info);
    delete info->session;
    map_session_.erase(itr);

}

void http_client::reset(session_info *info) {
    static const char *p = "HTTP/1.1 408 timeout\r\nServer: http session\r\nContent-Length:15\r\n\r\nconnection reset";
    static const int len = strlen(p);
    bcus::http_decoder d;
    d.decode(p, len);

    std::deque<session_info::req>::iterator itr = info->waiting_req_list.begin();
    for (; itr != info->waiting_req_list.end(); ++itr) {
        callback_(&d, bcus::endpoint(), itr->context);
    }
    info->waiting_req_list.clear();
}

}

