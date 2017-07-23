#ifndef BCUS_BLOCK_H
#define BCUS_BLOCK_H

#include <stdint.h>
#include <uint256.h>
#include "transaction.h"
#include "bignum.h"

class block_header
{
public:
    uint32_t version;
    uint64_t timestamp;
    uint256 hash_prev_block;
    uint256 hash_merkle_root;
    uint32_t bits;
    bignum nonce;

public:
    block_header();

    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        READWRITE(VARINT(version));
        READWRITE(VARINT(timestamp));
        READWRITE(hash_prev_block);
        READWRITE(hash_merkle_root);
        READWRITE(bits);
        READWRITE(nonce);
    }
};

class block : public block_header
{
public:
    std::vector<transaction_ptr> trans;

public:
    block();

    bool empty();
    void clear();

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void serialization(Stream& s, Operation ser_action)
    {
        block_header::serialization(s, ser_action);
        READWRITE(trans);
    }
};

#endif // BCUS_BLOCK_H
