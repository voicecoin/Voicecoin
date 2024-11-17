#include "addr_manager.h"
#include <cstring> 

namespace bcus {

static const char *strnstr(const char *haystack, const char *needle, int len) {
    int i;
    int needle_len;

    if (0 == (needle_len = strlen(needle)))
        return (char *)haystack;

    for (i=0; i<=(int)(len-needle_len); i++) {
        if ((haystack[0] == needle[0]) && (0 == strncmp(haystack, needle, needle_len)))
            return (char *)haystack;

        haystack++;
    }
    return NULL;
}

void addr_manager::http_read(bcus::http_decoder *d, const bcus::endpoint &ep, void *context)
{
    addr_callback_function *func = (addr_callback_function *)context;
    if (d->http_code() != 200) {
        XLOG(XLOG_WARNING, "addr_manager::%s, bad response, ep[%s]\n%s\n", ep.to_string().c_str(), d->to_string().c_str());
        (*func)("");
        return;
    }

    //<html><head><title>Current IP Check</title></head><body>Current IP Address: *.*.*.*</body></html>
    const bcus::slice body = d->body();
    const char *find = strnstr(body.c_str(), "Address:", body.length());
    char ip[32] = {0};
    if (1 != sscanf(find, "Address: %[^<]", ip)) {
        XLOG(XLOG_WARNING, "addr_manager::%s, bad body, ep[%s]\n%s\n", ep.to_string().c_str(), d->to_string().c_str());
    }
    (*func)(ip);
}

}
