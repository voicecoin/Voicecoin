#include "base58.h"
#include "hash.h"

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string base58::encode(const unsigned char* pbegin, const unsigned char* pend)
{
    int zeroes = 0;
    int length = 0;
    while (pbegin != pend && *pbegin == 0) {
        pbegin++;
        zeroes++;
    }

    int size = (pend - pbegin) * 138 / 100 + 1;
    std::vector<unsigned char> b58(size);
    while (pbegin != pend) {
        int carry = *pbegin;
        int i = 0;
        for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); (carry != 0 || i < length) && (it != b58.rend()); it++, i++) {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }
        length = i;
        pbegin++;
    }
    std::vector<unsigned char>::iterator it = b58.begin() + (size - length);
    while (it != b58.end() && *it == 0)
        it++;
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];
    return str;
}

std::string base58::encode(const std::vector<unsigned char>& vch)
{
    return encode(&vch[0], &vch[0] + vch.size());
}

bool base58::decode(const char* psz, std::vector<unsigned char>& vch)
{
    while (*psz && isspace(*psz))
        psz++;

    int zeroes = 0;
    int length = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }

    int size = strlen(psz) * 733 / 1000 + 1;
    std::vector<unsigned char> b256(size);

    while (*psz && !isspace(*psz)) {
        const char* ch = strchr(pszBase58, *psz);
        if (ch == NULL)
            return false;

        int carry = ch - pszBase58;
        int i = 0;
        for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); (carry != 0 || i < length) && (it != b256.rend()); ++it, ++i) {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        length = i;
        psz++;
    }

    while (isspace(*psz))
        psz++;
    if (*psz != 0)
        return false;

    std::vector<unsigned char>::iterator it = b256.begin() + (size - length);
    while (it != b256.end() && *it == 0)
        it++;

    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end())
        vch.push_back(*(it++));
    return true;
}

bool base58::decode(const std::string& str, std::vector<unsigned char>& ret)
{
    return decode(str.c_str(), ret);
}

std::string base58::encode_check(const std::vector<unsigned char>& vch)
{
    std::vector<unsigned char> vchRet(vch);
    uint256 hs = hash_helper::hash(vchRet.begin(), vchRet.end());
    vchRet.insert(vchRet.end(), (unsigned char*)&hs, (unsigned char*)&hs + 4);
    return encode(vchRet);
}

bool base58::decode_check(const char* psz, std::vector<unsigned char>& vch)
{
    if (!decode(psz, vch) ||
        (vch.size() < 4)) {
        vch.clear();
        return false;
    }

    uint256 hs = hash_helper::hash(vch.begin(), vch.end() - 4);
    if (memcmp(&hs, &vch.end()[-4], 4) != 0) {
        vch.clear();
        return false;
    }
    vch.resize(vch.size() - 4);
    return true;
}

bool base58::decode_check(const std::string& str, std::vector<unsigned char>& vch)
{
    return decode_check(str.c_str(), vch);
}