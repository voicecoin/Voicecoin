#include <stdexcept> 
#include "http_parser.h"
#include "loghelper.h"
namespace bcus {

http_decoder::http_decoder()
{
    reset();
}
void http_decoder::reset()
{
    seqno_ = 0;
    buf_ = NULL;
    len_ = 0;

    body_ = NULL;
    body_len_ = 0;

    header_.clear();

    memset(method_, 0, sizeof(method_));
    memset(url_path_, 0, sizeof(url_path_));

    http_version_ = 0;
    http_code_ = 0;
    memset(code_des_, 0, sizeof(code_des_));

    is_keep_alive_ = false;
    is_chunked_ = false;

    buf_body_.reset_loc(0);
}
const bcus::slice http_decoder::head(const char *k) const {
    HEADER::const_iterator i = header_.find(k);
    return i == header_.end() ? "" : i->second;
}
int http_decoder::decode(const char *buf, int len)
{
    //BS_XLOG(LOG_DEBUG, "http_decoder::%s, buf[%p], len[%d]\n", __FUNCTION__, buf, len);
    reset();
    buf_ = buf;
    len_ = len;

    if (0 == strncasecmp(buf, "HTTP/", 5)){
        is_request_ = false;
        return decode_response(buf, len);
    } else {
        is_request_ = true;
        return decode_request(buf, len);
    }
}

int http_decoder::decode_request(const char *buf, int len) {
    is_request_ = true;
    if (3 != sscanf(buf, "%[^ ] %s HTTP/1.%d", method_, url_path_, &http_version_)) {
        XLOG(XLOG_WARNING, "http_decoder::%s, bad buffer, len[%d]\n%s\n",
            __FUNCTION__, len, buf);
        return -1;
    }

    const char *buf_end = buf + len;

    const char *begin = strstr(buf, "\r\n");
    if (begin == NULL) {
        return 0;
    }

    begin += 2;
    const char *end = strstr(begin, "\r\n\r\n");
    if (end == NULL) {
        return 0;
    }

    end += 4;
    parse_head(begin, end - begin);

    if (end + body_len_ > buf_end) {
        return 0;
    }

    body_ = end;
    return end - buf + body_len_;
}

int http_decoder::decode_response(const char *buf, int len) {
    is_request_ = false;
    if (3 != sscanf(buf, "HTTP/1.%d %d %s", &http_version_, &http_code_, code_des_)) {
        XLOG(XLOG_WARNING, "http_decoder::%s, bad buffer, len[%d]\n%s\n",
            __FUNCTION__, len, buf);
        return -1;
    }

    const char *buf_end = buf + len;

    const char *begin = strstr(buf, "\r\n");
    if (begin == NULL) {
        return 0;
    }

    begin += 2;
    const char *end = strstr(begin, "\r\n\r\n");
    if (end == NULL) {
        return 0;
    }
    end += 4;
    parse_head(begin, end - begin);

    if(is_chunked_) {
        begin = end;
        while(begin < buf_end) {
            end = strstr(begin, "\r\n");
            if (end == NULL) {
                return 0;
            }
            end += 2;
            int chunklen = 0;
            if (1 != sscanf(begin, "%X", &chunklen) || chunklen < 0) {
                XLOG(XLOG_WARNING, "http_decoder::%s, bad chunklen, len[%d]\n%s\n",
                    __FUNCTION__, len, buf);
                return -1;
            }
            //BS_XLOG(LOG_DEBUG, "http_decoder::%s, chunklen[%d]\n", __FUNCTION__, chunklen);
            if (chunklen == 0) {
                body_ = buf_body_.base();
                body_len_ = buf_body_.len();
                end += 2;
                break;
            }
            if (end + chunklen + 2 > buf_end) { //incomplete packet  2 is \r\n
                return 0;
            }
            buf_body_.append(end, chunklen);
            begin = end + chunklen + 2;
        }
        return end - buf;
    }

    if (end + body_len_ > buf_end) {
        return 0;
    }

    body_ = end;
    return end - buf + body_len_;
}
int http_decoder::parse_head(const char *buf, int len) {
    const char *begin = buf;
    const char *buf_end  = buf + len;
    const char *end = strstr(begin, "\r\n");
    while (end != NULL && (end < buf_end)) {
        const char *colon = (const char *)memchr(begin, ':', end - begin);
        if (colon != NULL) {
            bcus::slice k(begin, colon - begin);
            bcus::slice v(colon + 1, end - colon -1);
            k.trim();
            v.trim();
            header_[k] = v;
            check_head(k ,v);
        }
        begin = end + 2;
        end = strstr(begin, "\r\n");
    }
    return 0;
}
void http_decoder::check_head(const bcus::slice &key, const bcus::slice &value)
{
    if (key == "Connection" && (value == "Keep-Alive" || value == "keep-alive")) {
        is_keep_alive_ = true;
    } else if(key == "Content-Length") {
        body_len_ = atoi(value.c_str());
    } else if(key == "Transfer-Encoding" && (value == "Chunked" || value == "chunked")) {
        is_chunked_ = true;
    } else if(key == "Set-Cookie") {
        //DecodeCookie(value.c_str(), value.length(), message);
    }
}

std::string http_decoder::to_string(int indent) const
{
    std::string str(indent*2, ' ');
    if (is_request_) {
        char sz[512] = {0};
        snprintf(sz, sizeof(sz) - 1, "%s %s HTTP/1.%d", method_, url_path_, http_version_);
        str.append(sz).append("\n");
    } else {
        char sz[512] = {0};
        snprintf(sz, sizeof(sz) - 1, "HTTP/1.%d %d %s", http_version_, http_code_, code_des_);
        str.append(sz).append("\n");
    }

    HEADER::const_iterator itr = header_.begin();
    for (; itr != header_.end(); ++itr)
    {
        str.append(",   ").append(itr->first.c_str(), itr->first.length())
           .append(":").append(itr->second.c_str(), itr->second.length())
           .append("\n");
    }
    if (body_) {
        str.append(body_, body_len_);
    }

    return str;
}



/*********************************************************/
/*********************************************************/
/*********************************************************/
static inline void http_time(time_t t, char out[128]) {
    struct tm gmt;
    gmtime_r(&t, &gmt);
    strftime(out, 127, "%a, %d %b %Y %H:%M:%S %Z", &gmt);
}
static inline const char *get_status(int code) {
    switch(code) {
        case 200: return "200 OK";
        case 201: return "201 Created";
        case 202: return "202 Accepted";
        case 204: return "204 No Content";
        case 300: return "300 Multiple Choices";
        case 301: return "301 Moved Permanently";
        case 302: return "302 Moved Temporarily";
        case 304: return "304 Not Modified";
        case 400: return "400 Bad Request";
        case 401: return "401 Unauthorized";
        case 403: return "403 Forbidden";
        case 404: return "404 Not Found";
        case 405: return "405 Method Not Allowed";
        case 500: return "500 Internal Server Error";
        case 501: return "501 Not Implemented";
        case 502: return "502 Bad Gateway";
        case 503: return "503 Service Unavailable";
        default:  return "Unknow Code";
    }
}
void http_encoder::init(const char *url, const char *method)
{
    if (0 != strncasecmp(url, "http://", 7)) {
        XLOG(XLOG_WARNING,"http_encodertor::%s, bad url protocol[%s]\n", __FUNCTION__, url);
        throw std::runtime_error("http_encoder::init, bad url protocol");
    }

    char domain[128] = {0};
    url += 7; //skip http://
    const char *path = strchr(url, '/');
    if (path == NULL) {
        path = "/";
        strcpy(domain, url);
    } else {
        strncpy(domain, url, path - url);
    }
    init(domain, path, method);
}
void http_encoder::init(const char *domain, const char *path, const char *method)
{
    buffer_.reset_loc(0);
    buffer_.append(method).append(" ").append(path).append(" ");
    buffer_.append("HTTP/1.1").append("\r\n");
    buffer_.append("Host: ").append(domain).append("\r\n");

    has_body_ = false;
}
void http_encoder::init(int http_code)
{
    buffer_.reset_loc(0);
    buffer_.append("HTTP/1.1 ").append(get_status(http_code)).append("\r\n");

    buffer_.append("Server: http_proxy").append("\r\n");

    has_body_ = false;
}
void http_encoder::head(const char *k, const char *v)
{
    buffer_.append(k).append(": ").append(v).append("\r\n");
}
void http_encoder::body(const char *body)
{
    http_encoder::body(body, strlen(body));
}
void http_encoder::body(const char *body, int len)
{
    has_body_ = true;
    char szlen[32] = {0};
    snprintf(szlen, sizeof(szlen) - 1, "%d", len);
    buffer_.append("Content-Length: ").append(szlen).append("\r\n");
    buffer_.append("\r\n");

    buffer_.append(body, len);
}

void http_encoder::encode()
{
    if (!has_body_) {
        buffer_.append("\r\n");
    }
}


}
