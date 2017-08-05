#ifndef BCUS_BLOCK_INFO_H
#define BCUS_BLOCK_INFO_H

#include "uint256.h"
#include "serialize.h"

class block_info
{
public:
    uint64_t timestamp;
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

public:
    block_tran_pos() : block_id(0), tran_pos(0) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(block_id);
        READWRITE(tran_pos);
    }
};

#endif // BCUS_BLOCK_INFO_H
