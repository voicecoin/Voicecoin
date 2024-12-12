#include "http_session.h"
#include "loghelper.h"
#include "channel_factory.h"

namespace bcus {
/***************************************************************/
/***************************************************************/
/***************************************************************/
class http_session::http_requester_type {
public:
    http_requester_type(boost::asio::io_service  &io_service, http_session *sess, channel_ptr channel);
    ~http_requester_type();
    void on_read(http_decoder *d, const bcus::endpoint &ep);
    int  do_request(uint32_t seqno, const void *buf, int len, void *context = NULL);
private:
    void check_timer();
private:
    boost::asio::io_service  &io_service_;
    channel_ptr channel_;
    bcus::io_timer timer_;
    http_session *session_;

    struct node {
        uint32_t seqno;
        uint32_t time;
        void *context;
        node() : seqno(0), time(0), context(NULL) {}
        node(uint32_t _seqno, uint32_t _time, void *_context) :
            seqno(_seqno), time(_time), context(_context) {}
    };
    typedef std::deque<node> history_type;
    history_type history_;
};

http_session::http_requester_type::http_requester_type(boost::asio::io_service  &io_service, http_session *sess, channel_ptr channel):
    io_service_(io_service), session_(sess)
{
    channel_ = channel;
    static const int CHECK_TIMER_INTERVAL = 1000; //1s
    timer_.init(&io_service_, CHECK_TIMER_INTERVAL,
        boost::bind(&http_requester_type::check_timer, this),
        bcus::io_timer::TIMER_CIRCLE);
    timer_.start();
}
http_session::http_requester_type::~http_requester_type()
{
    timer_.stop();
}
int http_session::http_requester_type::do_request(uint32_t seqno, const void *buf, int len, void *context)
{
    static const int TIMEOUT = 10; //10s
    history_.push_back(node(seqno, time(NULL) + TIMEOUT, context));
    channel_->async_write(buf, len);
    return 0;
}
void http_session::http_requester_type::on_read(http_decoder *d, const bcus::endpoint &ep)
{
    node nd = history_.front();
    history_.pop_front();
    d->seqno(nd.seqno);
    session_->read_callback_(session_, d, ep, nd.context);
}
void http_session::http_requester_type::check_timer()
{
    static const char *p = "HTTP/1.1 408 timeout\r\nServer: http session\r\nContent-Length:8\r\n\r\ntime out";
    static const int len = strlen(p);

    time_t now = time(NULL);
    /** if the request is timeout, empty the queue and close the session to avoid recv message later */
    if (!history_.empty() && now > history_.begin()->time) {
        bcus::http_decoder d;
        d.decode(p, len);
        while(!history_.empty()) {
            node nd = history_.front();
            history_.pop_front();
            d.seqno(nd.seqno);
            session_->read_callback_(session_, &d, channel_->remote_endpoint(), nd.context);
        }
        channel_->close();  //will call on_peer_close
    }
}
/***************************************************************/
/***************************************************************/
/***************************************************************/
class http_session::http_responser_type {
public:
    http_responser_type(boost::asio::io_service  &io_service, http_session *sess, channel_ptr channel);
    ~http_responser_type();
    void on_read(http_decoder *d, const bcus::endpoint &ep);
    int  do_response(uint32_t seqno, const void *buf, int len);
private:
    void check_timer();
private:
    boost::asio::io_service  &io_service_;
    channel_ptr channel_;
    bcus::io_timer timer_;
    http_session *session_;
    uint32_t mark_;

    struct node {
        uint32_t seqno;
        uint32_t time;
        bcus::buffer *buf;
        node() : seqno(0), time(0), buf(NULL) {}
        node(uint32_t _seqno, uint32_t _time) :
            seqno(_seqno), time(_time), buf(NULL) {}
        void append(const char *p, int len) {
            if (NULL == buf) {
                buf = new bcus::buffer();
            }
            buf->reset_loc(0);
            buf->append(p, len);
        }
    };
    typedef std::deque<node> history_type;
    history_type history_;
};


http_session::http_responser_type::http_responser_type(boost::asio::io_service  &io_service, http_session *sess, channel_ptr channel):
    io_service_(io_service), session_(sess), mark_(0)
{
    channel_ = channel;
    static const int CHECK_TIMER_INTERVAL = 1000; //1s
    timer_.init(&io_service_, CHECK_TIMER_INTERVAL,
        boost::bind(&http_responser_type::check_timer, this),
        bcus::io_timer::TIMER_CIRCLE);
    timer_.start();
}
http_session::http_responser_type::~http_responser_type()
{
    timer_.stop();
    history_type::iterator itr = history_.begin();
    for (; itr != history_.end(); ++itr) {
        if (itr->buf) {
            delete itr->buf;
            itr->buf = NULL;
        }
    }
}
void http_session::http_responser_type::on_read(http_decoder *d, const bcus::endpoint &ep)
{
    static const int TIMEOUT = 10; //10s
    uint32_t seqno = ++mark_;
    d->seqno(seqno);
    history_.push_back(node(seqno, time(NULL) + TIMEOUT));
    session_->read_callback_(session_, d, ep, NULL);
}
int  http_session::http_responser_type::do_response(uint32_t seqno, const void *buf, int len)
{
    bool send_succ = false;
    history_type::iterator itr = history_.begin();
    if (seqno == itr->seqno) {
        send_succ = true;
        channel_->async_write(buf, len);
        history_.pop_front();
        while(history_.end() != (itr = history_.begin())) {
            channel_->async_write(itr->buf->base(), itr->buf->len());
            delete itr->buf;
            history_.pop_front();
        }
    } else {
        for(++itr; itr != history_.end(); ++itr) {
            if(seqno == itr->seqno) {
                send_succ = true;
                itr->append((const char *)buf, len);
                break;
            }
        }
    }
    if (!send_succ) {
        XLOG(XLOG_WARNING, "http_session::%s, seqno[%u] has been dealed\n", __FUNCTION__, seqno);
    }
    return 0;
}

void http_session::http_responser_type::check_timer()
{
    static const char *p = "HTTP/1.1 504 timeout\r\nServer: http session\r\nContent-Length:8\r\n\r\ntime out";
    static const int len = strlen(p);

    time_t now = time(NULL);
    while(!history_.empty()){
        node &nd = history_.front();
        if (now < nd.time) {
            break;
        }
        if (nd.buf != NULL) {
            channel_->async_write(nd.buf->base(), nd.buf->len());
            delete nd.buf;
        } else {
            channel_->async_write(p, len);
        }
        history_.pop_front();
    }
}

/***************************************************************/
/***************************************************************/
/***************************************************************/

http_session::http_session( boost::asio::io_service &io_service, tcp_socket_ptr &socket) : io_service_(io_service)
{
    static const int STATUS_TIMEOUT_CHECK_INTERVAL = 30;  //30s

    XLOG(XLOG_DEBUG, "http_session::%s, fd[%d]\n", __FUNCTION__, socket->native_handle());
    channel_type *channel = new channel_type(
                                    io_service_,
                                    this,
                                    socket,
                                    STATUS_TIMEOUT_CHECK_INTERVAL);

    channel_.reset(channel);
    channel_->set_owner(this);
    channel_->async_read();

    http_requester_ = new http_requester_type(io_service, this, channel_);
    http_responser_ = new http_responser_type(io_service, this, channel_);
}
http_session::~http_session() {
    XLOG(XLOG_DEBUG, "http_session::%s\n", __FUNCTION__);
    delete http_requester_;
    delete http_responser_;
}
void http_session::on_read(bcus::http_decoder *d, const bcus::endpoint &ep)
{
    //XLOG(XLOG_DEBUG, "http_session::%s, %s\n", __FUNCTION__, d->to_string().c_str());
    if (d->is_request()) {
        http_responser_->on_read(d, ep);
    } else {
        http_requester_->on_read(d, ep);
    }
}
int  http_session::do_request(uint32_t seqno, const void *buf, int len, void *context)
{
    return http_requester_->do_request(seqno, buf, len, context);
}
int  http_session::do_response(uint32_t seqno, const void *buf, int len)
{
    return http_responser_->do_response(seqno, buf, len);
}
void http_session::on_peer_close() {
    close_callback_(this, channel_->remote_endpoint());
}
void http_session::close()
{
    channel_->set_owner(NULL);
    channel_->close();
}


}

