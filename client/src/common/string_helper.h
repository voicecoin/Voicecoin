#ifndef BCUS_COMMON_STRING_HELPER_H
#define BCUS_COMMON_STRING_HELPER_H
#include <string>
#include <string.h>
#include <vector>
#include "platform.h"

namespace bcus {

class string_helper {
public:
    static const char *strstr_comm(const char *str, int len);
    static const char *left_trim(const char *str, int len = 0);
    static const char *right_trim(const char *str, int len = 0);
    static const char *trim(char *str, int len = 0);
    static std::string str_trim(const char *str, int len = 0);
    static std::string str_to_lower(const char *str, int len = 0);
    static const char *get_next_token(char *buffer, char delim, char *&save_ptr);

    static void split(const std::string &line, char delim, std::vector<std::string > *ret);
    static std::string join(const std::vector<std::string> &vec, const char *delim);
    static std::string &replace(std::string &str, const char *src, const char *dest);

    static std::string format(const char *format, ...);
    //static
    inline static int copy(char *dst, const char *src) {
        /*
        char *p = dst;
        for (; *src != 0; ++src, ++p) {
            *p = *src;
        }
        *p = 0;
        return p - dst;
        */
        int len = strlen(src);
        memcpy(dst, src, len);
        //*(dst + len) = 0;
        return len;
    }


    template<typename T>
    static std::string to_hex(const T itbegin, const T itend, bool fSpaces=false)
    {
        std::string rv;
        static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                         '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
        rv.reserve((itend-itbegin)*3);
        for(T it = itbegin; it < itend; ++it)
        {
            unsigned char val = (unsigned char)(*it);
            if(fSpaces && it != itbegin)
                rv.push_back(' ');
            rv.push_back(hexmap[val>>4]);
            rv.push_back(hexmap[val&15]);
        }

        return rv;
    }

    template<typename T>
    static std::string to_hex(const T& vch, bool fSpaces=false)
    {
        return to_hex(vch.begin(), vch.end(), fSpaces);
    }

    static signed char hex_to_digit(char c);

    static std::string time_to_string(const char *format, time_t t);
};

}

#endif
