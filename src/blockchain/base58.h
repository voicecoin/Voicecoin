#pragma once

#include <vector>
#include <string>

namespace bcus {

class base58
{
public:
    static std::string encode(const unsigned char* pbegin, const unsigned char* pend);
    static std::string encode(const std::vector<unsigned char>& vch);

    static bool decode(const char* psz, std::vector<unsigned char>& ret);
    static bool decode(const std::string& str, std::vector<unsigned char>& ret);

    static std::string encode_check(const std::vector<unsigned char>& vch);

    static bool decode_check(const char* psz, std::vector<unsigned char>& ret);
    static bool decode_check(const std::string& str, std::vector<unsigned char>& ret);
};

}
