#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "hash.h"
#include "uint256.h"
#include "serialize.h"

class buff_stream
{
public:
    buff_stream() : read_pos_(0) {}
    buff_stream(const char* pbegin, const char* pend) : vec_(pbegin, pend), read_pos_(0) {}

    std::vector<unsigned char>::iterator begin() { return vec_.begin(); }

    std::vector<unsigned char>::iterator end() { return vec_.end(); }

    char *data() { return (char *)vec_.data() + read_pos_; }
    size_t size() { return vec_.size() - read_pos_; }
    void clear() { vec_.clear(); read_pos_ = 0; }

    void read(char* pch, int size)
    {
        if (size == 0) return;

        if (read_pos_ + size > vec_.size())
        {
            throw std::runtime_error("buff_stream::read(): end of data");
        }
        else if (read_pos_ + size == vec_.size())
        {
            memcpy(pch, &vec_[read_pos_], size);
            read_pos_ = 0;
            vec_.clear();
        }
        else
        {
            memcpy(pch, &vec_[read_pos_], size);
            read_pos_ += size;
        }
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

    template<typename T>
    buff_stream& operator>>(T& obj)
    {
        ::unserialize(*this, obj);
        return (*this);
    }

private:
    std::vector<unsigned char> vec_;
    size_t read_pos_;
};

template<typename T>
inline uint256 serialize_hash(T &t)
{
    buff_stream bs;
    bs << t;
    return hash_helper::hash(bs.begin(), bs.end());
}
