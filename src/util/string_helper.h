#ifndef STRING_HELPER_H
#define STRING_HELPER_H

#include <string>

class string_helper
{
public:
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

    static std::string time_to_string(const char *format, int64_t t);
};

#endif // STRING_HELPER_H
