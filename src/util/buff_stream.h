#ifndef BCUS_BUFF_STREAM_H
#define BCUS_BUFF_STREAM_H

#include <string>
#include <vector>
#include "hash.h"
#include "uint256.h"

class buff_stream
{
public:
    std::vector<unsigned char>::iterator begin()
    {
        return vec_.begin();
    }

    std::vector<unsigned char>::iterator end()
    {
        return vec_.end();
    }

    void write(const char* pch, int size)
    {
        vec_.insert(vec_.end(), pch, pch + size);
    }

    template<typename T>
    buff_stream& operator<<(const T& obj)
    {
        ::serialize(*this, obj);
        return (*this);
    }

private:
    std::vector<unsigned char> vec_;
};

template<typename T>
inline uint256 serialize_hash(T &t)
{
    buff_stream bs;
    bs << t;
    return hash_helper::hash(bs.begin(), bs.end());
}

#endif // BCUS_BUFF_STREAM_H
