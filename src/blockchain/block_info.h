#pragma once

#include "uint256.h"
#include "serialize.h"

namespace bcus {

class block_info
{
public:
    int64_t timestamp;
    uint32_t bits;
    uint32_t height;
    block_info *pre_info;
    uint256 *hash;

public:
    block_info() : timestamp(0), bits(0), height(0), pre_info(0) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(timestamp);
        READWRITE(bits);
        READWRITE(height);
    }
};

class block_tran_pos
{
public:
    int block_id;
    uint32_t tran_pos;
    std::vector<block_tran_pos> spents;

public:
    block_tran_pos(int block_id_ = 0, uint32_t tran_pos_ = 0)
        : block_id(block_id_), tran_pos(tran_pos_) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(block_id);
        READWRITE(tran_pos);
        READWRITE(spents);
    }

    bool operator==(const block_tran_pos& r) const
    {
        return block_id == r.block_id && tran_pos == r.tran_pos;
    }

    bool operator!=(const block_tran_pos& r) const
    {
        return !(*this == r);
    }
};

}