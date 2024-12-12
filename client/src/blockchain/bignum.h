#ifndef BCUS_BIG_NUM_H
#define BCUS_BIG_NUM_H

#include <stdexcept>
#include <vector>
#include <openssl/bn.h>
#include <cstdint>
#include <string>
#include <algorithm>
#include "uint256.h"
#if 0
class auto_bignum_ctx
{
protected:
    BN_CTX* pctx;
    BN_CTX* operator=(BN_CTX* pnew) { return pctx = pnew; }

public:
    auto_bignum_ctx()
    {
        pctx = BN_CTX_new();
        if (pctx == NULL)
            throw std::runtime_error("auto_bignum_ctx : BN_CTX_new() returned NULL");
    }

    ~auto_bignum_ctx()
    {
        if (pctx != NULL)
            BN_CTX_free(pctx);
    }

    operator BN_CTX*() { return pctx; }
    BN_CTX& operator*() { return *pctx; }
    BN_CTX** operator&() { return &pctx; }
    bool operator!() { return (pctx == NULL); }
};

class bignum : public BIGNUM
{
public:
    bignum()
    {
        BN_init(this);
    }

    bignum(const bignum& b)
    {
        BN_init(this);
        if (!BN_copy(this, &b))
        {
            BN_clear_free(this);
            throw std::runtime_error("bignum::bignum(const bignum&) : BN_copy failed");
        }
    }

    explicit bignum(const std::string& str)
    {
        BN_init(this);
        set_hex(str);
    }

    bignum& operator=(const bignum& b)
    {
        if (!BN_copy(this, &b))
            throw std::runtime_error("bignum::operator= : BN_copy failed");
        return (*this);
    }

    ~bignum()
    {
        BN_clear_free(this);
    }

    bignum(char n)             { BN_init(this); if (n >= 0) setulong(n); else setint64(n); }
    bignum(short n)            { BN_init(this); if (n >= 0) setulong(n); else setint64(n); }
    bignum(int n)              { BN_init(this); if (n >= 0) setulong(n); else setint64(n); }
    bignum(long n)             { BN_init(this); if (n >= 0) setulong(n); else setint64(n); }
    bignum(int64_t n)            { BN_init(this); setint64(n); }
    bignum(unsigned char n)    { BN_init(this); setulong(n); }
    bignum(unsigned short n)   { BN_init(this); setulong(n); }
    bignum(unsigned int n)     { BN_init(this); setulong(n); }
    bignum(unsigned long n)    { BN_init(this); setulong(n); }
    bignum(uint64_t n)           { BN_init(this); setuint64(n); }
    explicit bignum(uint256 n) { BN_init(this); setuint256(n); }

    explicit bignum(const std::vector<unsigned char>& vch)
    {
        BN_init(this);
        setvch(vch);
    }

    void setulong(unsigned long n)
    {
        if (!BN_set_word(this, n))
            throw std::runtime_error("bignum conversion from unsigned long : BN_set_word failed");
    }

    unsigned long getulong() const
    {
        return BN_get_word(this);
    }

    unsigned int getuint() const
    {
        return BN_get_word(this);
    }

    int getint() const
    {
        unsigned long n = BN_get_word(this);
        if (!BN_is_negative(this))
            return (n > INT_MAX ? INT_MAX : n);
        else
            return (n > INT_MAX ? INT_MIN : -(int)n);
    }

    void setint64(int64_t n)
    {
        unsigned char pch[sizeof(n) + 6];
        unsigned char* p = pch + 4;
        bool fNegative = false;
        if (n < (int64_t)0)
        {
            n = -n;
            fNegative = true;
        }
        bool fLeadingZeroes = true;
        for (int i = 0; i < 8; i++)
        {
            unsigned char c = (n >> 56) & 0xff;
            n <<= 8;
            if (fLeadingZeroes)
            {
                if (c == 0)
                    continue;
                if (c & 0x80)
                    *p++ = (fNegative ? 0x80 : 0);
                else if (fNegative)
                    c |= 0x80;
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        unsigned int nSize = p - (pch + 4);
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize) & 0xff;
        BN_mpi2bn(pch, p - pch, this);
    }

    void setuint64(uint64_t n)
    {
        unsigned char pch[sizeof(n) + 6];
        unsigned char* p = pch + 4;
        bool fLeadingZeroes = true;
        for (int i = 0; i < 8; i++)
        {
            unsigned char c = (n >> 56) & 0xff;
            n <<= 8;
            if (fLeadingZeroes)
            {
                if (c == 0)
                    continue;
                if (c & 0x80)
                    *p++ = 0;
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        unsigned int nSize = p - (pch + 4);
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize) & 0xff;
        BN_mpi2bn(pch, p - pch, this);
    }

    void setuint256(uint256 n)
    {
        unsigned char pch[sizeof(n) + 6];
        unsigned char* p = pch + 4;
        bool fLeadingZeroes = true;
        unsigned char* pbegin = (unsigned char*)&n;
        unsigned char* psrc = pbegin + sizeof(n);
        while (psrc != pbegin)
        {
            unsigned char c = *(--psrc);
            if (fLeadingZeroes)
            {
                if (c == 0)
                    continue;
                if (c & 0x80)
                    *p++ = 0;
                fLeadingZeroes = false;
            }
            *p++ = c;
        }
        unsigned int nSize = p - (pch + 4);
        pch[0] = (nSize >> 24) & 0xff;
        pch[1] = (nSize >> 16) & 0xff;
        pch[2] = (nSize >> 8) & 0xff;
        pch[3] = (nSize >> 0) & 0xff;
        BN_mpi2bn(pch, p - pch, this);
    }

    uint256 getuint256()
    {
        unsigned int nSize = BN_bn2mpi(this, NULL);
        if (nSize < 4)
            return 0;
        std::vector<unsigned char> vch(nSize);
        BN_bn2mpi(this, &vch[0]);
        if (vch.size() > 4)
            vch[4] &= 0x7f;
        uint256 n = 0;
        for (int i = 0, j = vch.size()-1; i < sizeof(n) && j >= 4; i++, j--)
            ((unsigned char*)&n)[i] = vch[j];
        return n;
    }

    void setvch(const std::vector<unsigned char>& vch)
    {
        std::vector<unsigned char> vch2(vch.size() + 4);
        unsigned int nSize = vch.size();
        vch2[0] = (nSize >> 24) & 0xff;
        vch2[1] = (nSize >> 16) & 0xff;
        vch2[2] = (nSize >> 8) & 0xff;
        vch2[3] = (nSize >> 0) & 0xff;
        reverse_copy(vch.begin(), vch.end(), vch2.begin() + 4);
        BN_mpi2bn(&vch2[0], vch2.size(), this);
    }

    std::vector<unsigned char> getvch() const
    {
        unsigned int nSize = BN_bn2mpi(this, NULL);
        if (nSize < 4)
            return std::vector<unsigned char>();
        std::vector<unsigned char> vch(nSize);
        BN_bn2mpi(this, &vch[0]);
        vch.erase(vch.begin(), vch.begin() + 4);
        reverse(vch.begin(), vch.end());
        return vch;
    }

    bignum& set_compact(unsigned int nCompact)
    {
        unsigned int nSize = nCompact >> 24;
        std::vector<unsigned char> vch(4 + nSize);
        vch[3] = nSize;
        if (nSize >= 1) vch[4] = (nCompact >> 16) & 0xff;
        if (nSize >= 2) vch[5] = (nCompact >> 8) & 0xff;
        if (nSize >= 3) vch[6] = (nCompact >> 0) & 0xff;
        BN_mpi2bn(&vch[0], vch.size(), this);
        return *this;
    }

    unsigned int get_compact() const
    {
        unsigned int nSize = BN_bn2mpi(this, NULL);
        std::vector<unsigned char> vch(nSize);
        nSize -= 4;
        BN_bn2mpi(this, &vch[0]);
        unsigned int nCompact = nSize << 24;
        if (nSize >= 1) nCompact |= (vch[4] << 16);
        if (nSize >= 2) nCompact |= (vch[5] << 8);
        if (nSize >= 3) nCompact |= (vch[6] << 0);
        return nCompact;
    }

    void set_hex(const std::string& str)
    {
        // skip 0x
        const char* psz = str.c_str();
        while (isspace(*psz))
            psz++;
        bool fNegative = false;
        if (*psz == '-')
        {
            fNegative = true;
            psz++;
        }
        if (psz[0] == '0' && tolower(psz[1]) == 'x')
            psz += 2;
        while (isspace(*psz))
            psz++;

        // hex string to bignum
        static char phexdigit[256] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0 };
        *this = 0;
        while (isxdigit(*psz))
        {
            *this <<= 4;
            int n = phexdigit[*psz++];
            *this += n;
        }
        if (fNegative)
            *this = 0 - *this;
    }

    template<typename Stream>
    void serialize(Stream& s) const
    {
        ::serialize(s, getvch());
    }

    template<typename Stream>
    void unserialize(Stream& s)
    {
        vector<unsigned char> vch;
        ::unserialize(s, vch);
        setvch(vch);
    }


    bool operator!() const
    {
        return BN_is_zero(this);
    }

    bignum& operator+=(const bignum& b)
    {
        if (!BN_add(this, this, &b))
            throw std::runtime_error("bignum::operator+= : BN_add failed");
        return *this;
    }

    bignum& operator-=(const bignum& b)
    {
        *this = *this - b;
        return *this;
    }

    bignum& operator*=(const bignum& b)
    {
        auto_bignum_ctx pctx;
        if (!BN_mul(this, this, &b, pctx))
            throw std::runtime_error("bignum::operator*= : BN_mul failed");
        return *this;
    }

    bignum& operator/=(const bignum& b)
    {
        *this = *this / b;
        return *this;
    }

    bignum& operator%=(const bignum& b)
    {
        *this = *this % b;
        return *this;
    }

    bignum& operator<<=(unsigned int shift)
    {
        if (!BN_lshift(this, this, shift))
            throw std::runtime_error("bignum:operator<<= : BN_lshift failed");
        return *this;
    }

    bignum& operator>>=(unsigned int shift)
    {
        if (!BN_rshift(this, this, shift))
            throw std::runtime_error("bignum:operator>>= : BN_rshift failed");
        return *this;
    }


    bignum& operator++()
    {
        // prefix operator
        if (!BN_add(this, this, BN_value_one()))
            throw std::runtime_error("bignum::operator++ : BN_add failed");
        return *this;
    }

    const bignum operator++(int)
    {
        // postfix operator
        const bignum ret = *this;
        ++(*this);
        return ret;
    }

    bignum& operator--()
    {
        // prefix operator
        bignum r;
        if (!BN_sub(&r, this, BN_value_one()))
            throw std::runtime_error("bignum::operator-- : BN_sub failed");
        *this = r;
        return *this;
    }

    const bignum operator--(int)
    {
        // postfix operator
        const bignum ret = *this;
        --(*this);
        return ret;
    }


    friend inline const bignum operator-(const bignum& a, const bignum& b);
    friend inline const bignum operator/(const bignum& a, const bignum& b);
    friend inline const bignum operator%(const bignum& a, const bignum& b);
};



inline const bignum operator+(const bignum& a, const bignum& b)
{
    bignum r;
    if (!BN_add(&r, &a, &b))
        throw std::runtime_error("bignum::operator+ : BN_add failed");
    return r;
}

inline const bignum operator-(const bignum& a, const bignum& b)
{
    bignum r;
    if (!BN_sub(&r, &a, &b))
        throw std::runtime_error("bignum::operator- : BN_sub failed");
    return r;
}

inline const bignum operator-(const bignum& a)
{
    bignum r(a);
    BN_set_negative(&r, !BN_is_negative(&r));
    return r;
}

inline const bignum operator*(const bignum& a, const bignum& b)
{
    auto_bignum_ctx pctx;
    bignum r;
    if (!BN_mul(&r, &a, &b, pctx))
        throw std::runtime_error("bignum::operator* : BN_mul failed");
    return r;
}

inline const bignum operator/(const bignum& a, const bignum& b)
{
    auto_bignum_ctx pctx;
    bignum r;
    if (!BN_div(&r, NULL, &a, &b, pctx))
        throw std::runtime_error("bignum::operator/ : BN_div failed");
    return r;
}

inline const bignum operator%(const bignum& a, const bignum& b)
{
    auto_bignum_ctx pctx;
    bignum r;
    if (!BN_mod(&r, &a, &b, pctx))
        throw std::runtime_error("bignum::operator% : BN_div failed");
    return r;
}

inline const bignum operator<<(const bignum& a, unsigned int shift)
{
    bignum r;
    if (!BN_lshift(&r, &a, shift))
        throw std::runtime_error("bignum:operator<< : BN_lshift failed");
    return r;
}

inline const bignum operator>>(const bignum& a, unsigned int shift)
{
    bignum r;
    if (!BN_rshift(&r, &a, shift))
        throw std::runtime_error("bignum:operator>> : BN_rshift failed");
    return r;
}

inline bool operator==(const bignum& a, const bignum& b) { return (BN_cmp(&a, &b) == 0); }
inline bool operator!=(const bignum& a, const bignum& b) { return (BN_cmp(&a, &b) != 0); }
inline bool operator<=(const bignum& a, const bignum& b) { return (BN_cmp(&a, &b) <= 0); }
inline bool operator>=(const bignum& a, const bignum& b) { return (BN_cmp(&a, &b) >= 0); }
inline bool operator<(const bignum& a, const bignum& b)  { return (BN_cmp(&a, &b) < 0); }
inline bool operator>(const bignum& a, const bignum& b)  { return (BN_cmp(&a, &b) > 0); }

#endif // BCUS_BIG_NUM_H
#endif