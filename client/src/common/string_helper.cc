#include "string_helper.h"
#include <cstring>
#include <stdarg.h>
#include "platform.h"

namespace bcus {

const char *string_helper::strstr_comm(const char *str, int len) {
    enum {BEGIN_REF = 0, END_REF, ESC};
    int state = -1;
    const char *end = str + len;
    for (const char *p = str; p < end; ++p) {
        switch(*p) {
          case '\"': {
            if (p > str && *(p - 1) == '\\') {  //ignore \"
                break;
            }
            state = (state == BEGIN_REF) ? END_REF : BEGIN_REF;
            break;
          }
          case ',' : {
            if (state != BEGIN_REF) {
              return p;
            }
          }
          default:
            break;
        }
    }
    return NULL;
  /*
    const char *end = str + len;
    if (*str == '\"') {  //ignore comma in ""
        const char *p = (const char *)memchr(str, ',', len);
        while(p != NULL && *(p - 1) != '\"') {
            ++p;
            p = (const char *)memchr(p, ',', end - p);
        }
        return p;
    } else {
        return (const char *)memchr(str, ',', len);
    }
  */
}

const char *string_helper::left_trim(const char *str, int len) {
    if (len == 0) {
        len = strlen(str);
    }
    const char *p = str;
    const char *end = str + len;
    for (; p < end && (*p == ' ' || *p == '\t'); ++p) { }
    return p;
}
const char *string_helper::right_trim(const char *str, int len ) {
    if (len == 0) {
        len = strlen(str);
    }
    const char *p = str + len - 1;
    for (; p >= str && (*p == ' ' || *p == '\t'); --p) { }
    return p;
}
const char *string_helper::trim(char *str, int len) {
    const char *p = left_trim(str, len);
    const char *q = right_trim(str, len);
    *((char *)q + 1) = 0;
    return p;
}
std::string string_helper::str_trim(const char *str, int len) {
    const char *p = left_trim(str, len);
    const char *q = right_trim(str, len);
    return std::string(p, q - p + 1);
}
std::string string_helper::str_to_lower(const char *str, int len) {
    if (len == 0) {
        len = strlen(str);
    }
    int n = 'A' - 'a';
    std::string out(str, len);
    for (std::size_t i = 0; i < out.size(); ++i) {
        if ('A' <= out[i] && out[i] <= 'Z') {
            out[i] -= n;
        }
    }
    return out;
}

const char *string_helper::get_next_token(char *buffer, char delim, char *&save_ptr) {
    char *from, *to;
    from = buffer;
    if(from == NULL) {
        save_ptr = NULL;
        return NULL;
    }

    bool has_quote = false;
    char quote_ch = '"';
    if (*from == quote_ch) {
        from++;
        has_quote = true;
    }
    to = from;

    while ((*to != '\0') && (*to != delim) && (*to != '\n')) {
        if (has_quote && (*to == quote_ch)) {
            to++;
            break;
        }
        to++;
    }

    save_ptr = (*to == delim) ? (to + 1) : NULL;

    if (!has_quote) {
        *to = '\0';
    }
    else {
        *(to-1) = '\0';
    }
    return from;
}

void string_helper::split(const std::string &line, char delim, std::vector<std::string > *ret) {
    std::string::size_type begin = 0, end = -1;
    while (std::string::npos != (end = line.find(delim, begin))) {
        ret->push_back(line.substr(begin, end - begin));
        begin = end + 1;
    }
    ret->push_back(line.substr(begin));
}
std::string string_helper::join(const std::vector<std::string> &vec, const char *delim) {
    std::string str;
    std::vector<std::string>::const_iterator itr = vec.begin();
    for (; itr != vec.end(); ++itr) {
        if (!str.empty()) {
            str.append(delim);
        }
        str.append(*itr);
    }
    return str;
}
std::string &string_helper::replace(std::string &str, const char *src, const char *dest) {
    size_t pos = 0;
    size_t len = strlen(src);
    while (std::string::npos != (pos = str.find(src, pos))) {
        str.replace(pos, len, dest);
    }
    return str;
}

std::string string_helper::format(const char *fmt, ...) {
    char sz[512] = {0};
    va_list va;
    va_start( va, fmt );
    vsnprintf( sz, sizeof(sz) - 1, fmt, va );
    va_end( va );
    return sz;
}


static const signed char p_hexdigit[256] =
{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,
-1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,0xa,0xb,0xc,0xd,0xe,0xf,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

signed char string_helper::hex_to_digit(char c)
{
    return p_hexdigit[(unsigned char)c];
}

std::string string_helper::time_to_string(const char *format, time_t t)
{
    char buf[128] = { 0 };
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(buf, 64, format, &tm);
    return buf;
}

}

